#include <stdio.h>
#include <string.h>
#include "my_uart_functions.h"  // Replace with the actual header file for your UART functions
#include "my_delay_functions.h" // Replace with the actual header file for your delay functions
#include "my_gpio_functions.h"  // Replace with the actual header file for your GPIO functions

#define MAX_RETRIES 5
#define TIMEOUT_MS 500
#define DEV_EUI_LENGTH 23

typedef enum {
    STATE_WAIT_FOR_BUTTON,
    STATE_CONNECT_TO_MODULE,
    STATE_CHECK_RESPONSE,
    STATE_READ_FIRMWARE,
    STATE_READ_DEVEUI
} State;

void removeColonsAndConvertToLower(char* input, char* output) {
    // Implement this function to remove colons and convert to lowercase
    // Example: input "2C:F7:F1:20:32:30:A5:70" -> output "2cf7f1203230a570"
}

int main() {
    State state = STATE_WAIT_FOR_BUTTON;
    int retries = 0;

    while (1) {
        switch (state) {
            case STATE_WAIT_FOR_BUTTON:
                if (!gpio_get(PIN_SW0)) {
                    while (!gpio_get(PIN_SW0)) {
                        sleep_ms(50);
                    }
                    state = STATE_CONNECT_TO_MODULE;
                }
                break;

            case STATE_CONNECT_TO_MODULE:
                uart_send_command("AT");
                delay_ms(TIMEOUT_MS);

                if (uart_has_response()) {
                    printf("Connected to LoRa module\n");
                    state = STATE_CHECK_RESPONSE;
                    retries = 0;
                } else {
                    retries++;
                    if (retries >= MAX_RETRIES) {
                        printf("Module not responding\n");
                        state = STATE_WAIT_FOR_BUTTON;
                    }
                }
                break;

            case STATE_CHECK_RESPONSE:
                if (uart_has_response()) {
                    // Process response as needed
                    state = STATE_READ_FIRMWARE;
                } else {
                    printf("Module stopped responding\n");
                    state = STATE_WAIT_FOR_BUTTON;
                }
                break;

            case STATE_READ_FIRMWARE:
                uart_send_command("AT+VERSION?");
                delay_ms(TIMEOUT_MS);

                if (uart_has_response()) {
                    // Process firmware version response as needed
                    state = STATE_READ_DEVEUI;
                } else {
                    printf("Module stopped responding\n");
                    state = STATE_WAIT_FOR_BUTTON;
                }
                break;

            case STATE_READ_DEVEUI:
                uart_send_command("AT+DEVEUI?");
                delay_ms(TIMEOUT_MS);

                if (uart_has_response()) {
                    char response[DEV_EUI_LENGTH + 1];
                    uart_read_response(response, DEV_EUI_LENGTH);
                    response[DEV_EUI_LENGTH] = '\0';

                    char processedDevEui[DEV_EUI_LENGTH / 2];
                    removeColonsAndConvertToLower(response, processedDevEui);

                    printf("Processed DevEui: %s\n", processedDevEui);
                } else {
                    printf("Module stopped responding\n");
                }
                state = STATE_WAIT_FOR_BUTTON;
                break;
        }
    }

    return 0;
}