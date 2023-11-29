#include "my_pico_definitions.h"

#define MY_I2C_BAUD_RATE 100000 // Baudrate of I2C in Hz (e.g. 100kHz is 100000)
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

#define DEVADDR 0x50

struct LedStates led_states;   // Create an instance of the led states structure
struct SwitchStates sw_states; // Create an instance of the structure and a void pointer

int main()
{
    stdio_init_all();
    initialize_all_sw_buttons();
    initialize_all_leds();

    // I2C
    i2c_init(i2c0, MY_I2C_BAUD_RATE); // Either i2c0 or i2c1, baudrate in HZ, Returns Actual set baudrate
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    sw_states.switch0 = 1; // mb dont need to set it?
    sw_states.switch1 = 1;
    sw_states.switch2 = 1;

    sw_states.sw_changed = false;

    sw_states.sw_all_toggle = true; // toggle for debouncing

    void *ptr_current_sw_states = &sw_states;

    static struct repeating_timer my_timer; // https://www.raspberrypi.com/documentation//pico-sdk/high_level.html#rpip8e15c0ded1c0628069f4
    my_timer.user_data = NULL;              // to here pass data to store in the repeating_timer structure for use by the callback, 3rd parameter in add_repeating_timer_ms
    static struct repeating_timer *ptr_my_timer = &my_timer;
    add_repeating_timer_ms(10, rt_callback_function_sw, ptr_current_sw_states, ptr_my_timer);

    uint8_t data_to_write[4] = {};

    // Memory address divided into two 8-bit parts
    const uint16_t memory_slot = 0x7FFE; // 0x7FFF is highest possible
    data_to_write[0] = (memory_slot >> 8) & 0xFF;
    data_to_write[1] = memory_slot & 0xFF;

    uint8_t buf[2];

    i2c_write_blocking(i2c0, DEVADDR, data_to_write, 2, false);
    sleep_ms(10);
    i2c_read_blocking(i2c0, DEVADDR, buf, 2, false);
    led_states.state = buf[0];
    led_states.not_state = buf[1];
    print_binary(buf[0]);
    print_binary(buf[1]);

    // Check led state to turn on or off leds
    if (led_state_is_valid(&led_states))
    {
        gpio_put(PIN_LED1, (led_states.state & 0x01));
        gpio_put(PIN_LED2, (led_states.state >> 1) & 0x01);
        gpio_put(PIN_LED3, (led_states.state >> 2) & 0x01);
    }
    else
    {
        led_states.state = 0b010;

        gpio_put(PIN_LED1, (led_states.state & 0x01));
        gpio_put(PIN_LED2, (led_states.state >> 1) & 0x01);
        gpio_put(PIN_LED3, (led_states.state >> 2) & 0x01);
    }
    printf("| %d | %d | %d | ", (led_states.state & 0x01), ((led_states.state >> 1) & 0x01), ((led_states.state >> 2) & 0x01));
    print_time_stamp_s();

    while (true)
    {
        if (sw_states.sw_changed)
        {

            data_to_write[2] = led_states.state;
            led_states.not_state = ~led_states.state;
            data_to_write[3] = led_states.not_state;
            print_binary(led_states.state);
            print_binary(led_states.not_state);
            i2c_write_blocking(i2c0, DEVADDR, data_to_write, 4, false);
            sleep_ms(10);

            printf("| %d | %d | %d | ", (led_states.state & 0x01), ((led_states.state >> 1) & 0x01), ((led_states.state >> 2) & 0x01));
            print_time_stamp_s();

            gpio_put(PIN_LED1, (led_states.state & 0x01));
            gpio_put(PIN_LED2, (led_states.state >> 1) & 0x01);
            gpio_put(PIN_LED3, (led_states.state >> 2) & 0x01);
            sw_states.sw_changed = false;
        }

        // Updating states on press of button
        if (sw_states.switch0 && sw_states.switch1 && sw_states.switch2)
        {
            sw_states.sw_all_toggle = true;
        }

        if (sw_states.sw_all_toggle && (!sw_states.switch0 || !sw_states.switch1 || !sw_states.switch2))
        {
            sw_states.sw_all_toggle = false;
            if (!sw_states.switch0 && !sw_states.sw_changed)
            {
                led_states.state ^= 0b100;
                sw_states.sw_changed = true;
            }
            if (!sw_states.switch1 && !sw_states.sw_changed)
            {
                led_states.state ^= 0b010;
                sw_states.sw_changed = true;
            }
            if (!sw_states.switch2 && !sw_states.sw_changed)
            {
                led_states.state ^= 0b001;
                sw_states.sw_changed = true;
            }
        }
    }
}