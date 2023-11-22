#include "my_pico_definitions.h"

#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define STRLEN 50            // 528
#define MY_TIMEOUT_US 500000 // in microseconds
#define MAX_ATTEMPTS 5
#define RESPONSE_AT "+AT: OK"



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


}
