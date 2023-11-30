#include "my_pico_definitions.h"

// I2C Configuration
#define MY_I2C_BAUD_RATE 100000
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

// EEPROM Configuration
#define DEVADDR 0x50
#define MEMORY_ADDR_LED_STATES 0x7FFE // Second-highest memory address in EEPROM

// Constants for Bitwise Operations
#define MASK_8bit_11111111 0xFF

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
    write_buf[0] = (memory_slot >> 8) & MASK_8bit_11111111; // Memory address divided into two 8-bit parts
    write_buf[1] = memory_slot & MASK_8bit_11111111;

    i2c_write_blocking(i2c0, DEVADDR, write_buf, 2, false);
    i2c_read_blocking(i2c0, DEVADDR, read_buf, 2, false);
    led_states.state = read_buf[1];     // State is the last (task requirement)
    led_states.not_state = read_buf[0]; // Inverse of state goes first

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

    // Main Loop
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
    }
}
