#include "my_pico_definitions.h"

// #define MY_I2C_BAUD_RATE 100000
// int main()
// {
//     stdio_init_all();
//     initialize_all_sw_buttons();
//     initialize_all_leds();
//     initialize_pwm_pin(PIN_LED3);
//     i2c_init(i2c0, MY_I2C_BAUD_RATE); // Either i2c0 or i2c1, baudrate	Baudrate in Hz (e.g. 100kHz is 100000) Returns Actual set baudrate
// }

int main()
{
    const uint led_pin = 22;
    uint count = 0;
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    stdio_init_all();
    cyw43_arch_init();

    while (true)
    {
        printf("Blink! %u\r\n", ++count);
        // gpio_put(led_pin, true);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(1000);
        // gpio_put(led_pin, false);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(1000);
    }
}