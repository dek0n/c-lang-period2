#include "my_pico_definitions.h"

#define CMD_READ "read"
#define CMD_ERASE "erase"
#define GETCHAR_TIMEOUT_US 0 // small timeout (5) may needed for working with interrupt handler, now testing without it

typedef enum
{
    STATE_IDLE = 1,
    STATE_INPUT = 2,
    STATE_READ = 3,
    STATE_ERASE = 4,
} ProgramStatesLab5;

// Globals
bool my_input_toggle = false;
ProgramStatesLab5 program_state = STATE_IDLE;

// Interrupt handler for input mode
void my_interrupt_handler_input_mode()
{
    if (!my_input_toggle)
    {
        my_input_toggle = true;
        program_state = STATE_INPUT;
    }
}

// LED and Switch States Structures
struct LedStates led_states;   // Instance of the LED states structure
struct SwitchStates sw_states; // Instance of the switch states structure

int main()
{
    // Initialization
    stdio_init_all();
    initialize_all_sw_buttons();
    initialize_all_leds();

    // I2C Initialization
    i2c_init(i2c0, MY_I2C_BAUD_RATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    // Variables
    char my_character;
    int my_character_int;
    char my_character_array[MY_STRING_LENGTH_MAX];
    int my_input_index = 0;
    char *formed_string;

    // Interrupts
    gpio_set_irq_enabled_with_callback(UART_RX_PIN, GPIO_IRQ_EDGE_RISE, true, &my_interrupt_handler_input_mode);

    // Timer for updating states buttons
    void *ptr_current_sw_states = &sw_states;
    static struct repeating_timer my_timer;
    my_timer.user_data = NULL;
    static struct repeating_timer *ptr_my_timer = &my_timer;
    add_repeating_timer_ms(10, rt_callback_function_sw, ptr_current_sw_states, ptr_my_timer);

    // Initialization of Switch States
    sw_states.sw_changed = false;

    // Buffers for I2C to EEPROM write and read
    uint8_t write_buf[4];
    uint8_t read_buf[2];
    char log_string[61];

    // Reading from EEPROM on power-up
    const uint16_t memory_slot = MEMORY_ADDR_LED_STATES;
    write_buf[0] = (memory_slot >> 8) & MASK_8B_ALL1; // Memory address divided into two 8-bit parts
    write_buf[1] = memory_slot & MASK_8B_ALL1;

    i2c_write_blocking(i2c0, DEVADDR, write_buf, 2, false);
    sleep_ms(10);
    i2c_read_blocking(i2c0, DEVADDR, read_buf, 2, false);
    led_states.state = read_buf[1];     // State is the last (task requirement)
    led_states.not_state = read_buf[0]; // Inverse of state goes first

    memset(log_string, 0, sizeof(log_string)); // clearing the log_string
    // Writing 'boot' to the log on power-up
    memcpy(log_string, "boot", strlen("boot"));
    log_string[4] = '\0';
    write_to_log(log_string);                  // writing 'boot' to log
    sleep_ms(10);
    memset(log_string, 0, sizeof(log_string)); // clearing the log_string

    // Applying starting states
    if (led_state_is_valid(&led_states)) // Comparing state to inverse state to validate reading from memory; otherwise, default states
    {
        update_leds_from_led_states(&led_states); // Update LEDs
    }
    else
    {
        led_states.state = 0b010;                 // Default state of LEDs
        update_leds_from_led_states(&led_states); // Update LEDs
    }

    // Printing initial states
    print_led_states(&led_states);
    print_time_stamp_s();

    // MAIN LOOP
    while (true)
    {

        switch (program_state) // SWITCH for program sates
        {
        case STATE_IDLE:
            // Cleaning input buffer
            clean_getchar_buffer();
            // turning input interrupt  ON
            gpio_set_irq_enabled_with_callback(UART_RX_PIN, GPIO_IRQ_EDGE_RISE, true, &my_interrupt_handler_input_mode);
            while (program_state == STATE_IDLE)
            {
                // Updating toggle on release of a button
                if (sw_states.switch0 && sw_states.switch1 && sw_states.switch2)
                {
                    sw_states.sw_all_toggle = true;
                }

                // On change of states actions:
                if (sw_states.sw_changed)
                {
                    update_leds_from_led_states(&led_states); // Update LEDs

                    // Writing new states to EEPROM
                    write_buf[3] = led_states.state; // State is the last (task requirement)
                    led_states.not_state = ~led_states.state;
                    write_buf[2] = led_states.not_state; // Inverse state goes before the last
                    i2c_write_blocking(i2c0, DEVADDR, write_buf, 4, false);

                    // Printing states
                    print_led_states(&led_states);
                    print_time_stamp_s();

                    // Writing to log
                    formed_string = form_led_states(&led_states);
                    memcpy(log_string, formed_string, sizeof(log_string));
                    write_to_log(log_string);                  // writing led states to the log
                    memset(log_string, 0, sizeof(log_string)); // clearing the log_string
                    memset(write_buf, 0, sizeof(write_buf));   // clearing the write_buf
                    free(formed_string);
                    sw_states.sw_changed = false;
                    sleep_ms(50); // ??
                }
                // Changing  sw states
                if (sw_states.sw_all_toggle && (!sw_states.switch0 || !sw_states.switch1 || !sw_states.switch2))
                {
                    sw_states.sw_all_toggle = false;
                    if (!sw_states.switch0 && !sw_states.sw_changed)
                    {
                        led_states.state ^= 0b100; // Inversion of single bit with mask
                    }
                    if (!sw_states.switch1 && !sw_states.sw_changed)
                    {
                        led_states.state ^= 0b010; // Inversion of single bit with mask
                    }
                    if (!sw_states.switch2 && !sw_states.sw_changed)
                    {
                        led_states.state ^= 0b001; // Inversion of single bit with mask
                    }
                    sw_states.sw_changed = true;
                }
                break;
            }
            // turning input interrupt OFF
            gpio_set_irq_enabled_with_callback(UART_RX_PIN, GPIO_IRQ_EDGE_RISE, false, &my_interrupt_handler_input_mode);
            break;

        case STATE_INPUT:
            // Input mode toggled by interrupt
            while (my_input_toggle)
            {
                my_character = my_character_int = getchar_timeout_us(GETCHAR_TIMEOUT_US);
                if (my_character_int != PICO_ERROR_TIMEOUT)
                {
                    switch (my_character_int) // SWITCH (nested) for different input characters
                    {
                    case ASCII_ENTER:
                        my_character_array[my_input_index] = '\0';
                        printf("\n");
                        if (!strcmp(my_character_array, CMD_READ))
                        {

                            program_state = STATE_READ;
                        }
                        else if (!strcmp(my_character_array, CMD_ERASE))
                        {

                            program_state = STATE_ERASE;
                        }
                        else
                        {
                            printf("INPUT ERROR!\n");
                            program_state = STATE_IDLE;
                        }
                        my_input_index = 0;
                        my_input_toggle = false;
                        break;

                    case ASCII_ESC:
                        if (my_input_index > 0)
                        {
                            printf("\r");
                            for (int i = 0; i < my_input_index; i++)
                            {
                                printf(" ");
                            }
                            printf("\r");
                        }
                        my_input_index = 0;
                        my_input_toggle = false;
                        program_state = STATE_IDLE;
                        break;

                    case ASCII_BACKSPACE:
                        if (my_input_index > 0)
                        {
                            my_character_array[my_input_index] = ' ';
                            my_input_index--;
                            printf("\b \b");
                        }
                        break;

                    default:
                        if (my_character_int != ASCII_BACKSPACE && my_input_index < MY_STRING_LENGTH_MAX)
                        {
                            my_character_array[my_input_index] = my_character;
                            my_input_index++;
                            printf("%c", (char)my_character);
                        }
                        break;
                    }
                }
            }
            break;

        case STATE_READ:
            read_from_log();

            // if nothing is availabale print that there is nothing
            // reading the whole log from min adress o to max if available

            // uint8_t buffer[10] = {51, 32, 93, 84, 75, 16, 17, 28};
            // uint16_t crc = crc16(buffer, 8);
            // // put CRC after data
            // buffer[8] = (uint8_t)(crc >> 8);
            // buffer[9] = (uint8_t)crc;
            // // validate data
            // if (crc16(buffer, 10) != 0)
            // {
            //     printf("Error\n");
            // }

            program_state = STATE_IDLE;
            break;

        case STATE_ERASE:
            printf("Erasing log\n");
            erase_log();
            program_state = STATE_IDLE;
            break;
        }
    }
}
