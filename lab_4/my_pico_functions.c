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

bool led_state_is_valid(ledstate *ls) // Validation of stored in EEPROM ledstate by comparing original with inverted.
{                                     // Typecast to uint8_t is needed for the compare to work correctly because operand of bitwise not gets promoted to an integer. Typecast to 8-bit value discards the extra bits that got added in the promotion.
    return ls->state == (uint8_t)~ls->not_state;
}

void set_led_state(ledstate *ls, uint8_t value) // helper function that sets value and inverted value of state in a structure
{
    ls->state = value;
    ls->not_state = ~value;
}

void print_binary(uint8_t value) {
    for (int i = 7; i >= 0; i--) {
        printf("%c", (value & (1 << i)) ? '1' : '0');
    }
    printf("\n");
}