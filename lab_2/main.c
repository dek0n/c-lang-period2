#include "my_pico_definitions.h"
struct SwitchStates sw_states; // Create an instance of the structure and a void pointer

void my_interrupt_handler()
{
    sw_states.rot_turn_toggle = true;
    if (gpio_get(PIN_ROTB))
    {
        sw_states.rot_turn_direction = 0;
    }
    else
    {
        sw_states.rot_turn_direction = 1;
    }
};

int main()
{
    stdio_init_all();

    initialize_all_leds();
    initialize_all_sw_buttons();
    initialize_pwm_pin(PIN_LED1);
    initialize_pwm_pin(PIN_LED2);
    initialize_pwm_pin(PIN_LED3);

    // Initializing rotary pins ,onfigure ROTA and ROTB as an input without pull-up/pull-down, ROT_Push with pull up
    gpio_init(PIN_ROT_Push);
    gpio_set_dir(PIN_ROT_Push, GPIO_IN);
    gpio_pull_up(PIN_ROT_Push);

    gpio_init(PIN_ROTA);
    gpio_set_dir(PIN_ROTA, GPIO_IN);

    gpio_init(PIN_ROTB);
    gpio_set_dir(PIN_ROTA, GPIO_IN);

    // Interrupts
    gpio_set_irq_enabled_with_callback(PIN_ROTA, GPIO_IRQ_EDGE_RISE, true, &my_interrupt_handler);

    sw_states.switch0 = 1; // mb dont need to set it?
    sw_states.switch1 = 1;
    sw_states.switch2 = 1;
    sw_states.rot_push = 1;
    sw_states.sw_all_toggle = true; // toggle for debouncing

    sw_states.rot_turn_toggle = false;
    sw_states.rot_turn_direction = 0;

    void *ptr_current_sw_states = &sw_states;

    static struct repeating_timer my_timer; // https://www.raspberrypi.com/documentation//pico-sdk/high_level.html#rpip8e15c0ded1c0628069f4
    my_timer.user_data = NULL;              // to here pass data to store in the repeating_timer structure for use by the callback, 3rd parameter in add_repeating_timer_ms
    static struct repeating_timer *ptr_my_timer = &my_timer;
    add_repeating_timer_ms(10, rt_callback_function_sw, ptr_current_sw_states, ptr_my_timer);

    //  array of 76 values of brightness
    int br_values_array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 18, 19, 21, 22, 24, 25, 27, 29, 32, 34, 36, 39, 42, 45, 48, 51, 55, 59, 64, 68, 73, 78, 84, 90, 97, 103,
                             111, 119, 128, 137, 147, 157, 168, 181, 194, 207, 222, 238, 256, 274, 294, 315, 337, 362, 388, 415, 445, 477, 512, 548, 588, 630, 675, 724, 776, 831, 891, 955, 1000};
    //  brlev  is brightness level
    // if brlev_MAX, _MID, _MIN are not multiples of step, need to add extra measures for handling extreme values
    int brlev_MAX = 75;
    int brlev_MID = 35;
    int brlev_MIN = 0;
    int brlev_step_sw = 1;
    int brlev_step_rot = 5;
    int brlev_current = brlev_MID; // strarting level from 0 to 75

    int led_brightness = br_values_array[brlev_current]; // starting brightness

    // Toggles

    bool sw1_button_pressed = false; // Starting state of the power button
    bool sw1_button_toggle = true;
    bool led_state = false; // Switched off at start

    while (true)
    {
        // Check led state to turn on or off leds
        if (led_state)
            all_leds_on(led_brightness);
        else
            all_leds_off();

        // Smoothly change brightness (debounced by toggling sw_all_toggle)
        if (led_state)
        {
            if (!gpio_get(PIN_SW0) && brlev_current < brlev_MAX && sw_states.sw_all_toggle)
            {
                brlev_current += brlev_step_sw;
                led_brightness = br_values_array[brlev_current];
                sw_states.sw_all_toggle = false;
            }
            if (!gpio_get(PIN_SW2) && brlev_current > brlev_MIN && sw_states.sw_all_toggle)
            {
                brlev_current -= brlev_step_sw;
                led_brightness = br_values_array[brlev_current];
                sw_states.sw_all_toggle = false;
            }
            // Rotary version
            if (sw_states.rot_turn_toggle)
            {
                if (sw_states.rot_turn_direction && brlev_current < brlev_MAX - brlev_step_rot)
                {
                    brlev_current += brlev_step_rot;
                    if (brlev_current > brlev_MAX - brlev_step_rot) // if step is not 1, this fix is needed
                    {
                        brlev_current = brlev_MAX;
                    }
                    led_brightness = br_values_array[brlev_current];
                }
                if (!sw_states.rot_turn_direction && brlev_current >= brlev_MIN + brlev_step_rot)
                {
                    brlev_current -= brlev_step_rot;
                    if (brlev_current < brlev_MIN + brlev_step_rot) // if step is no 1, this fix is needed
                    {
                        brlev_current = brlev_MIN;
                    }
                    led_brightness = (br_values_array[brlev_current]);
                }
                sw_states.rot_turn_toggle = false;
            }
        }

        // Updating power button
        if (!sw_states.switch1 || !sw_states.rot_push) // if pressed, if toggle allowed, preessed is true, restrict toggle
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
                if (led_brightness != 0)
                    led_state = false;
                else
                {
                    led_state = true;
                    brlev_current = brlev_MID; // if brightness was 0 when turned off it is turned on on the MID brightness
                    led_brightness = (br_values_array[brlev_current]);
                }
            }
            else
            {
                led_state = true;
            }
        }
    }
}