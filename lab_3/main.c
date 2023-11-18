#include "my_pico_definitions.h"

#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define STRLEN 50 // 528

int main()
{
    stdio_init_all();
    initialize_all_sw_buttons();

    // Set up our UART with the required speed.
    uart_init(uart0, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // See datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    const uint8_t send[] = "AT+VER\n";
    char str[STRLEN];
    int pos = 0;

    while (1)
    {

        if (!gpio_get(PIN_SW0))
        {
            printf("sw0 pressed\n");
            while (!gpio_get(PIN_SW0))
            {
                sleep_ms(50);
            }
            uart_write_blocking(uart0, send, strlen((const char *)send)); // Write to the UART for transmission. This function will block until all the data has been sent to the UART
        }
        while (uart_is_readable(uart0)) // Determine whether data is waiting in the RX FIFO.
        {
            char c = uart_getc(uart0);
            if (c == '\r' || c == '\n')
            {
                str[pos] = '\0';
                printf("received: %s\n", str);
                pos = 0; // start over after line is printed
            }
            else
            {
                if (pos < STRLEN - 1)
                {
                    str[pos++] = c;
                }
            }
        }
    }
}
