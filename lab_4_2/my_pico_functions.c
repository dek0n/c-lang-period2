#include "my_pico_definitions.h"

void initialize_pwm_pin(int pin_for_pwm) // Configure PWM for selected pin
{
    uint slice = pwm_gpio_to_slice_num(pin_for_pwm);
    uint channel = pwm_gpio_to_channel(pin_for_pwm);
    pwm_set_enabled(slice, false);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, PWM_CLOCK_DIVIDER);
    pwm_config_set_wrap(&config, PWM_CHANNEL_TOP);
    pwm_init(slice, &config, false);
    pwm_set_chan_level(slice, channel, PWM_CHANNEL_COMPARE_LEVEL);
    gpio_set_function(pin_for_pwm, GPIO_FUNC_PWM);
    pwm_set_enabled(slice, true);
}

bool rt_callback_function_sw(struct repeating_timer *rt) // Callback function for repeating timer to check status of SWs
{
    struct SwitchStates *ptr_switch_states = (struct SwitchStates *)rt->user_data; // Cast the user_data pointer to struct SwitchStates*

    if (ptr_switch_states) // Check if the pointer is not NULL to ensure safety
    {
        ptr_switch_states->switch0 = gpio_get(PIN_SW0); // Modify
        ptr_switch_states->switch1 = gpio_get(PIN_SW1);
        ptr_switch_states->switch2 = gpio_get(PIN_SW2);
        ptr_switch_states->rot_push = gpio_get(PIN_ROT_Push);
    }
    return true;
}

void initialize_all_leds() // Initialize all 3 LEDs at once
{
    gpio_init(PIN_LED1);
    gpio_set_dir(PIN_LED1, GPIO_OUT);
    gpio_init(PIN_LED2);
    gpio_set_dir(PIN_LED2, GPIO_OUT);
    gpio_init(PIN_LED3);
    gpio_set_dir(PIN_LED3, GPIO_OUT);
}

void initialize_all_sw_buttons() // Initialize all 3 SW buttons at once
{
    gpio_init(PIN_SW0);
    gpio_set_dir(PIN_SW0, GPIO_IN);
    gpio_pull_up(PIN_SW0);
    gpio_init(PIN_SW1);
    gpio_set_dir(PIN_SW1, GPIO_IN);
    gpio_pull_up(PIN_SW1);
    gpio_init(PIN_SW2);
    gpio_set_dir(PIN_SW2, GPIO_IN);
    gpio_pull_up(PIN_SW2);
}

void all_leds_on(int led_brightness)
{
    pwm_set_gpio_level(PIN_LED1, led_brightness);
    pwm_set_gpio_level(PIN_LED2, led_brightness);
    pwm_set_gpio_level(PIN_LED3, led_brightness);
}

void all_leds_off()
{
    pwm_set_gpio_level(PIN_LED1, 0);
    pwm_set_gpio_level(PIN_LED2, 0);
    pwm_set_gpio_level(PIN_LED3, 0);
}

void process_DevEui(const char *devEuiWithPrefix, char *result)
{
    const char *devEui = devEuiWithPrefix + strlen("+ID: DevEui, ");

    int resultIndex = 0;
    for (int i = 0; devEui[i] != '\0'; ++i)
    {
        if (isxdigit((unsigned char)devEui[i]))
        {
            result[resultIndex++] = tolower((unsigned char)devEui[i]);
        }
        else if (devEui[i] == ':')
        {
            continue;
        }
    }
    result[resultIndex] = '\0';
}

void clear_uart_buffer(uart_inst_t *uart)
{
    while (uart_is_readable(uart))
        uart_getc(uart);
}

void print_time_stamp_s()
{
    int x = time_us_64() / 1000000;
    printf("%ds since powerup\n", x);
}

void print_led_states(ledstate *ls)
{
    printf("| %d | %d | %d | ", (ls->state & 0x01), ((ls->state >> 1) & 0x01), ((ls->state >> 2) & 0x01));
}

char *form_led_states(ledstate *ls)
{
    int x = time_us_64() / 1000000;
    char *string = (char *)malloc(61 * sizeof(char)); // Allocate memory
    if (string == NULL)
    {
        // Handle allocation failure if needed
        return NULL;
    }
    sprintf(string, "| %d | %d | %d | %ds since powerup.", (ls->state & 0x01), ((ls->state >> 1) & 0x01), ((ls->state >> 2) & 0x01), x);
    return string;
}

bool led_state_is_valid(ledstate *ls) // Validation of stored in EEPROM ledstate by comparing original with inverted.
{                                     // Typecast to uint8_t is needed for the compare to work correctly because operand of bitwise not gets promoted to an integer. Typecast to 8-bit value discards the extra bits that got added in the promotion.
    return ls->state == (uint8_t)~ls->not_state;
}

void set_led_state(ledstate *ls, uint8_t value) // helper function that sets value and inverted value of state in a structure
{
    ls->state = value;
    ls->not_state = ~value;
}

void print_binary(uint8_t value)
{
    for (int i = 7; i >= 0; i--)
    {
        printf("%c", (value & (1 << i)) ? '1' : '0');
    }
    printf("\n");
}

void update_leds_from_led_states(ledstate *ls)
{
    gpio_put(PIN_LED1, (ls->state & 0x01));
    gpio_put(PIN_LED2, (ls->state >> 1) & 0x01);
    gpio_put(PIN_LED3, (ls->state >> 2) & 0x01);
}

void my_configure_motor()
{
    // Configure MOTOR
    gpio_init(PIN_OPTO_FORK);
    gpio_set_dir(PIN_OPTO_FORK, GPIO_IN);
    gpio_pull_up(PIN_OPTO_FORK);

    gpio_init(PIN_PIEZO_SENSOR);
    gpio_set_dir(PIN_PIEZO_SENSOR, GPIO_IN);
    gpio_pull_up(PIN_PIEZO_SENSOR);

    gpio_init(PIN_MOTOR_1);
    gpio_set_dir(PIN_MOTOR_1, GPIO_OUT);

    gpio_init(PIN_MOTOR_2);
    gpio_set_dir(PIN_MOTOR_2, GPIO_OUT);

    gpio_init(PIN_MOTOR_3);
    gpio_set_dir(PIN_MOTOR_3, GPIO_OUT);

    gpio_init(PIN_MOTOR_4);
    gpio_set_dir(PIN_MOTOR_4, GPIO_OUT);
}

void motor_turn_off_coils()
{
    gpio_put(PIN_MOTOR_1, 0);
    gpio_put(PIN_MOTOR_2, 0);
    gpio_put(PIN_MOTOR_3, 0);
    gpio_put(PIN_MOTOR_4, 0);
}

void motor_step(int in1, int in2, int in3, int in4)
{
    gpio_put(PIN_MOTOR_1, in1);
    gpio_put(PIN_MOTOR_2, in2);
    gpio_put(PIN_MOTOR_3, in3);
    gpio_put(PIN_MOTOR_4, in4);
    busy_wait_us(MOTOR_STEP_DELAY_US);
}

void clean_getchar_buffer()
{
    while (1)
    {
        int c = getchar_timeout_us(0);

        if (c == PICO_ERROR_TIMEOUT || c == PICO_ERROR_GENERIC)
        {
            break; // Exit the loop when there are no more characters to read
        }
    }
}

uint16_t crc16(const uint8_t *data_p, size_t length)
{
    uint8_t x;
    uint16_t crc = 0xFFFF;
    while (length--)
    {
        x = crc >> 8 ^ *data_p++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
    }
    return crc;
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
            sleep_ms(10);

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
                sleep_ms(10);
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