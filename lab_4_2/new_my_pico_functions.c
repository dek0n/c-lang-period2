void initialize_3_leds(int pin_led_1, int pin_led_2, int pin_led_3)
{
    gpio_init(pin_led_1);
    gpio_set_dir(pin_led_1, GPIO_OUT);
    gpio_init(pin_led_2);
    gpio_set_dir(pin_led_2, GPIO_OUT);
    gpio_init(pin_led_3);
    gpio_set_dir(pin_led_3, GPIO_OUT);
}

void initialize_3_buttons(int pin_button_1, int pin_button_2, int pin_button_3)
{
    gpio_init(pin_button_1);
    gpio_set_dir(pin_button_1, GPIO_IN);
    gpio_pull_up(pin_button_1);
    gpio_init(pin_button_2);
    gpio_set_dir(pin_button_2, GPIO_IN);
    gpio_pull_up(pin_button_2);
    gpio_init(pin_button_3);
    gpio_set_dir(pin_button_3, GPIO_IN);
    gpio_pull_up(pin_button_3);
}


void write_to_log(char *string)
{
    if (strlen(string) > 61)
    {
        printf("String Error.\n");
    }
    else
    {
        bool entry_done = false;
        uint16_t memory_slot = MEMORY_ADDR_LOG_START;
        uint8_t buffer[2];
        uint8_t memory_slot_status;
        uint8_t log_entry[66];
        uint8_t read_buf[1];
        while (!entry_done)
        {

            buffer[0] = memory_slot >> 8;
            buffer[1] = memory_slot;
            i2c_write_blocking(i2c0, DEVADDR, buffer, sizeof(buffer), false);

            // reading if the address is empty
            i2c_read_blocking(i2c0, DEVADDR, read_buf, 1, false);
            memory_slot_status = read_buf[0];

            // if address is empty write entry there
            if (memory_slot_status == 0)
            {
                log_entry[0] = memory_slot >> 8;
                log_entry[1] = memory_slot;
                int string_length = strlen(string);
                // Start copying the string to log_entry at index 3
                for (int i = 0; i < string_length; i++)
                {
                    log_entry[i + 2] = string[i];
                }
                log_entry[string_length + 2] = '\0'; // Add a null terminator at the end
                log_entry[string_length + 3] = 'C';  // Add CRC!
                log_entry[string_length + 4] = 'C';  // Add CRC!
                i2c_write_blocking(i2c0, DEVADDR, log_entry, sizeof(log_entry), false);
                entry_done = true;
                break;
            }
            // if address is full add 64 to memory_slot and read again
            else
            {
                if (memory_slot < MEMORY_ADDR_LOG_END)
                {
                    memory_slot = memory_slot + 0x40;
                }
                else
                {
                    erase_log();
                    memory_slot = MEMORY_ADDR_LOG_START;
                }
            }
        }
    }
}

void read_from_log()
{
    uint16_t memory_slot = MEMORY_ADDR_LOG_START;
    uint8_t buffer[2];
    uint8_t read_buf[1];
    int log_line = 1;

    while (memory_slot != MEMORY_ADDR_LOG_END)
    {
        buffer[0] = memory_slot >> 8;
        buffer[1] = memory_slot;
        i2c_write_blocking(i2c0, DEVADDR, buffer, sizeof(buffer), false);

        // Reading the memory slot status, which is also the first character of the log entry
        i2c_read_blocking(i2c0, DEVADDR, read_buf, 1, false);
        char character_to_print = read_buf[0];

        if (character_to_print != 0) // Check if the log entry is not empty
        {
            printf("LOG %d. %c", log_line, character_to_print); // Print line number and first character

            while (character_to_print != '\0') // Continue reading until null terminator is reached
            {
                i2c_read_blocking(i2c0, DEVADDR, read_buf, 1, false);
                character_to_print = read_buf[0];

                if (character_to_print != '\0')
                {
                    printf("%c", character_to_print);
                }
            }
            printf("\n");
            log_line++;
        }

        if (memory_slot < MEMORY_ADDR_LOG_END)
        {
            memory_slot += 0x40; // Move to the next log entry
        }
    }
}

void erase_log()
{
    uint8_t buffer[3];
    for (uint16_t i = MEMORY_ADDR_LOG_START; i != MEMORY_ADDR_LOG_END; i = i + MEMORY_SIZE_LOG_ENTRY)
    {
        buffer[0] = i >> 8;
        buffer[1] = i;
        buffer[2] = 0x00;
        i2c_write_blocking(i2c0, DEVADDR, buffer, sizeof(buffer), false);
        sleep_ms(10);
    }
}