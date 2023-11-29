#include "my_pico_definitions.h"

#define MY_I2C_BAUD_RATE 100000 // Baudrate of I2C in Hz (e.g. 100kHz is 100000)
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

#define DEVADDR 0x50

int main()
{
    stdio_init_all();
    initialize_all_sw_buttons();
    initialize_all_leds();
    i2c_init(i2c0, MY_I2C_BAUD_RATE); // Either i2c0 or i2c1, baudrate in HZ, Returns Actual set baudrate
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    uint8_t data_to_write[4] = {};
    const uint16_t memory_slot = 0x0100;

    // Divide into two 8-bit parts
    data_to_write[0] = (memory_slot >> 8) & 0xFF;
    data_to_write[1] = memory_slot & 0xFF;
    data_to_write[2] = 0x18;
    data_to_write[3] = 0x17;

    uint8_t data_read[2];

    i2c_write_blocking(i2c0, DEVADDR, data_to_write, 4, false);
    sleep_ms(10);

    i2c_write_blocking(i2c0, DEVADDR, data_to_write, 2, false);
    i2c_read_blocking(i2c0, DEVADDR, data_read, 2, false);
    printf("%s\n", "------");
    printf("%02X\n", data_read[0]);
    printf("%02X\n", data_read[1]);
    printf("%s\n", "      ");


    for (size_t i = 0; i < 2; i++)
    {
        if (data_read[i] == data_to_write[i + 2])
        {
            printf("Write and read successful.\n");
        }
        else
        {
            printf("Error: Read value does not match the written value.\n");
        }
    }
}
