#include "my_pico_definitions.h"

#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define STRLEN 50            // 528
#define MY_TIMEOUT_US 500000 // in microseconds
#define MAX_ATTEMPTS 5

#define RESPONSE_AT "+AT: OK"

typedef enum
{
    STATE_WAIT_FOR_BUTTON = 1,
    STATE_CONNECT_TO_MODULE = 2,
    STATE_READ_FIRMWARE = 3,
    STATE_READ_DEVEUI = 4
} State;

State state = STATE_WAIT_FOR_BUTTON;

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

    const uint8_t message_AT[] = "AT\n";
    const uint8_t message_firmaware[] = "AT+VER\n";
    const uint8_t message_DevEui[] = "AT+ID=DevEui\n";
    char response_string[STRLEN];
    char response_DevEui[17]; 
    int pos = 0;
    int connection_attempts = 0;
    bool connection_status = false;

    while (1)
    {
    exitLoops:
        switch (state)
        {
        /* 1. Program waits for user to press SW_0. When user presses the button then program starts communication with the LoRa module.*/
        case STATE_WAIT_FOR_BUTTON:
            if (!gpio_get(PIN_SW0))
            {
                while (!gpio_get(PIN_SW0))
                {
                    sleep_ms(50);
                }
                state = STATE_CONNECT_TO_MODULE;
            }
            break;

        case STATE_CONNECT_TO_MODULE:
            uart_write_blocking(uart0, message_AT, strlen((const char *)message_AT)); // Write to the UART for transmission. This function will block until all the data has been sent to the UART
            sleep_ms(50);
            connection_attempts = 0;
            connection_status = false;
            while (connection_attempts < 5)
            {
                if (uart_is_readable_within_us(uart0, MY_TIMEOUT_US))
                {
                    connection_attempts = 5;
                    while (uart_is_readable(uart0)) // Determine whether data is waiting in the RX FIFO.
                    {
                        char c = uart_getc(uart0);
                        if (c == '\n' || c == '\r')
                        {
                            response_string[pos] = '\0';
                            if (!strcmp(response_string, RESPONSE_AT))
                            {
                                printf("Connected to LoRa module.\n");
                                pos = 0; // start over after line is printed
                                connection_status = true;
                                while (!uart_getc(uart0))
                                    ; // clearing buffer
                                state = STATE_READ_FIRMWARE;
                                goto exitLoops;
                            }
                            else
                            {
                                printf("ERROR, received: '%s'\n", response_string);
                                pos = 0; // start over after line is printed
                                connection_status = false;
                                while (!uart_getc(uart0))
                                    ; // clearing buffer
                                state = STATE_WAIT_FOR_BUTTON;
                                break;
                            }
                        }
                        else
                        {
                            if (pos < STRLEN - 1)
                            {
                                response_string[pos++] = c;
                            }
                        }
                    }
                }
                else
                {
                    connection_attempts += 1;
                    printf("connection_attempts %d\n", connection_attempts);
                }
            }
            if (!connection_status)
            {
                printf("module not responding\n");
                state = STATE_WAIT_FOR_BUTTON;
            }
            state = STATE_WAIT_FOR_BUTTON;
            break;

        case STATE_READ_FIRMWARE:
            uart_write_blocking(uart0, message_firmaware, strlen((const char *)message_firmaware)); // Write to the UART for transmission. This function will block until all the data has been sent to the UART
            sleep_ms(50);
            if (uart_is_readable_within_us(uart0, MY_TIMEOUT_US))
            {
                while (uart_is_readable(uart0)) // Determine whether data is waiting in the RX FIFO.
                {
                    char c = uart_getc(uart0);
                    if (c == '\n' || c == '\r')
                    {
                        response_string[pos] = '\0';
                        printf("%s\n", response_string);
                        pos = 0; // start over after line is printed
                        while (!uart_getc(uart0))
                            ;                               // clearing buffer
                        memset(response_string, 0, STRLEN); // clearing response string maybe not needed?
                        state = STATE_READ_DEVEUI;
                        break;
                    }
                    else
                    {
                        if (pos < STRLEN - 1)
                        {
                            response_string[pos++] = c;
                        }
                    }
                }
            }
            else
            {
                printf("Module stopped responding\n");
                state = STATE_WAIT_FOR_BUTTON;
            }
            break;

        case STATE_READ_DEVEUI:
            uart_write_blocking(uart0, message_DevEui, strlen((const char *)message_DevEui)); // Write to the UART for transmission. This function will block until all the data has been sent to the UART
            sleep_ms(50);
            if (uart_is_readable_within_us(uart0, MY_TIMEOUT_US))
            {
                while (uart_is_readable(uart0)) // Determine whether data is waiting in the RX FIFO.
                {
                    char c = uart_getc(uart0);
                    if (c == '\n' || c == '\r')
                    {
                        response_string[pos] = '\0';
                        processDevEui(response_string, response_DevEui);
                        printf("%s\n", response_DevEui);
                        pos = 0; // start over after line is printed
                        while (!uart_getc(uart0))
                            ;                               // clearing buffer
                        memset(response_string, 0, STRLEN); // clearing response string maybe not needed?
                        state = STATE_WAIT_FOR_BUTTON;
                        break;
                    }
                    else
                    {
                        if (pos < STRLEN - 1)
                        {
                            response_string[pos++] = c;
                        }
                    }
                }
            }
            else
            {
                printf("Module stopped responding\n");
                state = STATE_WAIT_FOR_BUTTON;
            }
            break;
        }
    }
    return 0;
}

/*2. Program message_to_sends command “AT” to module and waits for response for 500 ms. If no response is
received or the response is not correct the program tries again up to five times. If no response is
received after five attempts program prints “module not responding” and goes back to step 1. If
response is received program prints “Connected to LoRa module”.*/

/*3. Program reads firmware version of the module and prints the result. If no response is received in
500 ms program prints “Module stopped responding” and goes back to step 1.*/

/*4. Program reads DevEui from the device. If no response is received in 500 ms program prints
“Module stopped responding” and goes back to step 1. DevEui contains 8 bytes that the module
outputs in hexadecimal separated by colons. The program must remove the colons between the
bytes and convert the hexadecimal digits to lower case.*/

/*5. Go to step 1*/
