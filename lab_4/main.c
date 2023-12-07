#include "my_pico_definitions.h"

// I2C Configuration
#define MY_I2C_BAUD_RATE 100000
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

// EEPROM Configuration
#define DEVADDR 0x50
#define MEMORY_ADDR_LED_STATES 0x7FFE // Second-highest memory address in EEPROM

// Constants for Bitwise Operations
#define MASK_8B_ALL1 0xFF

// For input
#define ASCII_SPACE 32
#define ASCII_ENTER 13
#define ASCII_BACKSPACE 8
#define MY_INPUT_DEALY_MS 100
#define MY_ARRAY_LENGTH 5
#define MY_ARRAY_READ           \
    {                           \
        'r', 'e', 'a', 'd', ' ' \
    }
#define MY_ARRAY_ERASE          \
    {                           \
        'e', 'r', 'a', 's', 'e' \
    }

// LED and Switch States Structures
struct LedStates led_states;   // Instance of the LED states structure
struct SwitchStates sw_states; // Instance of the switch states structure

bool my_compare_int_arrays(int *array_1, int *array_2, int size)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        if (array_1[i] != array_2[i])
        {
            break;
        }
    }
    // Check if the loop completed without a difference
    if (i == 5)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void print_my_array(int *arr)
{
    for (int i = 0; i != MY_ARRAY_LENGTH + 1; i++)
    {
        printf("%c", arr[i]);
    }
}

void reset_input_array(int *array, int length)
{
    for (int i = 0; i < length; ++i)
    {
        array[i] = ASCII_SPACE;
    }
}

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

    // Reading from EEPROM on power-up
    const uint16_t memory_slot = MEMORY_ADDR_LED_STATES;
    write_buf[0] = (memory_slot >> 8) & MASK_8B_ALL1; // Memory address divided into two 8-bit parts
    write_buf[1] = memory_slot & MASK_8B_ALL1;

    i2c_write_blocking(i2c0, DEVADDR, write_buf, 2, false);
    i2c_read_blocking(i2c0, DEVADDR, read_buf, 2, false);
    led_states.state = read_buf[1];     // State is the last (task requirement)
    led_states.not_state = read_buf[0]; // Inverse of state goes first

    // int my_character;

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

    // For input handling
    int my_character;
    int my_character_array[MY_ARRAY_LENGTH] = {[0 ... 4] = ASCII_SPACE};
    bool my_input_loop = false;
    int my_input_index = 0;
    int my_array_read[] = MY_ARRAY_READ;
    int my_array_erase[] = MY_ARRAY_ERASE;

    // MAIN LOOP LEVEL 1
    while (true)
    {
        // Updating toggle on release of a button
        if (sw_states.switch0 && sw_states.switch1 && sw_states.switch2)
        {
            sw_states.sw_all_toggle = true;
        }

        if (sw_states.sw_changed)
        {
            update_leds_from_led_states(&led_states); // Update LEDs

            // Writing new states to EEPROM
            write_buf[3] = led_states.state; // State is the last (task requirement)
            led_states.not_state = ~led_states.state;
            write_buf[2] = led_states.not_state; // Inverse state goes first
            i2c_write_blocking(i2c0, DEVADDR, write_buf, 4, false);

            // Printing states
            print_led_states(&led_states);
            print_time_stamp_s();

            sw_states.sw_changed = false;
        }

        if (sw_states.sw_all_toggle && (!sw_states.switch0 || !sw_states.switch1 || !sw_states.switch2))
        {
            sw_states.sw_all_toggle = false;
            if (!sw_states.switch0 && !sw_states.sw_changed)
            {
                led_states.state ^= 0b100; // Inversion of single bit with mask
                sw_states.sw_changed = true;
            }
            if (!sw_states.switch1 && !sw_states.sw_changed)
            {
                led_states.state ^= 0b010; // Inversion of single bit with mask
                sw_states.sw_changed = true;
            }
            if (!sw_states.switch2 && !sw_states.sw_changed)
            {
                led_states.state ^= 0b001; // Inversion of single bit with mask
                sw_states.sw_changed = true;
            }
        }

        // INPUT HANDLING LOOP LEVEL 2
        my_character = getchar_timeout_us(0);
        if (my_character != PICO_ERROR_TIMEOUT && my_character != ASCII_ENTER && my_character != ASCII_BACKSPACE)
        {
            my_character_array[my_input_index] = my_character;
            my_input_index++;
            printf("INPUT MODE:%c", (char)my_character);
            my_input_loop = true;

            while (my_input_loop)
            {
                my_character = getchar_timeout_us(0);
                if (my_character != PICO_ERROR_TIMEOUT)
                {
                    if (my_character == ASCII_ENTER)
                    {
                        printf("\n");
                        if (my_compare_int_arrays(my_character_array, my_array_read, MY_ARRAY_LENGTH))
                        {
                            printf("READING LOG...\n");
                        }
                        else if (my_compare_int_arrays(my_character_array, my_array_erase, MY_ARRAY_LENGTH))
                        {
                            printf("ERASING LOG...\n");
                        }
                        else
                        {
                            printf("INPUT ERROR! Available commands:'erase','read'.");
                        }
                        my_input_loop = false;
                        printf("\n");
                        reset_input_array(&my_character_array, MY_ARRAY_LENGTH);
                        my_input_index = 0;
                        my_input_loop = false;
                    }

                    else if (my_character == ASCII_BACKSPACE && my_input_index > 0)
                    {
                        my_character_array[my_input_index] = ASCII_SPACE;
                        printf("\b \b");
                        my_input_index--;
                    }

                    else
                    {
                        if (my_character != ASCII_BACKSPACE && my_input_index < 5)
                        {

                            my_character_array[my_input_index] = my_character;
                            my_input_index++;
                            printf("%c", (char)my_character);
                        }
                    }
                    sleep_ms(MY_INPUT_DEALY_MS); // delay to reduce CPU usage in input mode
                }
            }
        }
    }
}
