#include "my_pico_definitions.h"

#define MY_I2C_BAUD_RATE 100000 // Baudrate of I2C in Hz (e.g. 100kHz is 100000)
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

#define DEVADDR 0xA0

uint8_t data_to_write = 0xA5;
uint8_t *ptr_data_to_write = &data_to_write;
uint8_t data_read;

int main()
{
    stdio_init_all();
    initialize_all_sw_buttons();
    initialize_all_leds();
    i2c_init(i2c0, MY_I2C_BAUD_RATE); // Either i2c0 or i2c1, baudrate in HZ, Returns Actual set baudrate
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

    i2c_write_blocking(i2c0, DEVADDR, ptr_data_to_write, 2, false);
}
