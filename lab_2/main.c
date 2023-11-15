#include "my_pico_definitions.h"

int main()
{
    stdio_init_all();

    initialize_all_leds();
    initialize_all_sw_buttons();
    initialize_pwm_pin(PIN_LED1);
    initialize_pwm_pin(PIN_LED2);
    initialize_pwm_pin(PIN_LED3);

    struct SwitchStates current_sw_states; // Create an instance of the structure and a void pointer
    current_sw_states.switch0 = 1;         // mb dont need to set it?
    current_sw_states.switch1 = 1;
    current_sw_states.switch2 = 1;
    current_sw_states.sw_all_toggle = true; // toggle for debouncing
    // current_sw_states.rotary_push = 1;
    void *ptr_current_sw_states = &current_sw_states;

    static struct repeating_timer my_timer; // https://www.raspberrypi.com/documentation//pico-sdk/high_level.html#rpip8e15c0ded1c0628069f4
    my_timer.user_data = NULL;              // to here pass data to store in the repeating_timer structure for use by the callback, 3rd parameter in add_repeating_timer_ms
    static struct repeating_timer *ptr_my_timer = &my_timer;
    add_repeating_timer_ms(10, rt_callback_function_sw, ptr_current_sw_states, ptr_my_timer);

    // int filter_counter = 0;
    float led_brightness_level = 5.0; // strarting level from 0 to 10
    float led_brightness_level_MAX = 10.0;
    float led_brightness_level_MIN = 0.0;

    int led_brightness_value = (pow(2, led_brightness_level) - 1); // starting brightness

    bool sw1_button_pressed = false; // Starting state of the power button
    bool sw1_button_toggle = true;
    bool led_state = false; // Switched off at start

    while (true)
    {
        // Check led state to turn on or off leds
        if (led_state)
            all_leds_on(led_brightness_value);
        else
            all_leds_off();

        // Smoothly change brightness (debounced by toggling sw_all_toggle)
        if (led_state)
        {
            if (!gpio_get(PIN_SW0) && led_brightness_level < led_brightness_level_MAX && current_sw_states.sw_all_toggle)
            {
                led_brightness_level += 0.1;
                led_brightness_value = (pow(2, led_brightness_level) - 1);
                if (led_brightness_value > MAX_LED_BRIGHTNESS)
                {
                    led_brightness_value = MAX_LED_BRIGHTNESS;
                }
                current_sw_states.sw_all_toggle = false;
            }
            if (!gpio_get(PIN_SW2) && led_brightness_value > led_brightness_level_MIN && current_sw_states.sw_all_toggle)
            {
                led_brightness_level -= 0.1;
                led_brightness_value = (pow(2, led_brightness_level) - 1);
                current_sw_states.sw_all_toggle = false;
            }
        }

        // Updating power button
        if (!current_sw_states.switch1) // if pressed, if toggle allowed, preessed is true, restrict toggle
        {
            if (sw1_button_toggle)
            {
                sw1_button_pressed = true;
                sw1_button_toggle = false;
            }
        }

        else
        {
            sw1_button_pressed = false; // not pressed (released) allow toggle again
            sw1_button_toggle = true;
        }

        if (sw1_button_pressed) // if button pressed, restrict toggle, unmark pressed and make actions
        {
            sw1_button_toggle = false;
            sw1_button_pressed = false;
            if (led_state)
            {
                if (led_brightness_value != 0)
                    led_state = false;
                else
                {
                    led_state = true;
                    led_brightness_level = 5;
                    led_brightness_value = (pow(2, led_brightness_level) - 1);
                }
            }
            else
            {
                led_state = true;
            }
        }
    }
}