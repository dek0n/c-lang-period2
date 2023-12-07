#include "my_pico_definitions.h"

// I2C Configuration
#define MY_I2C_BAUD_RATE 100000
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

// EEPROM Configuration
#define DEVADDR 0x50
#define MEMORY_ADDR_LED_STATES 0x7FFE // Second-highest memory address in EEPROM

// Constants for Bitwise Operations
#define MASK_8B_ALL1 0xFF

// For input
#define ASCII_SPACE 32
#define ASCII_ENTER 13
#define ASCII_BACKSPACE 8
#define MY_INPUT_DEALY_MS 100
#define MY_STRING_LENGTH_MAX 10

#define STRING_STATUS "status"
#define STRING_CALIB "calib"
#define STRING_RUN "run "

bool my_input_toggle = false;
void my_interrupt_handler()
{
    if (!my_input_toggle)
    {
        my_input_toggle = true;
    }
}

int main()
{

    // Initialization
    stdio_init_all();
    initialize_all_sw_buttons();
    initialize_all_leds();

    // I2C Initialization
    i2c_init(i2c0, MY_I2C_BAUD_RATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    char my_character;
    int my_character_int;
    char my_character_array[MY_STRING_LENGTH_MAX];
    bool my_input_loop = false;
    int my_input_index = 0;
    int my_number_of_runs = 0;

    // Interrupts
    gpio_set_irq_enabled_with_callback(1, GPIO_IRQ_EDGE_RISE, true, &my_interrupt_handler);

    printf("Press ENTER for input mode.\n");
    // MAIN LOOP
    while (true)
    {
        if (my_input_toggle)
        {
            my_input_toggle = false;
            my_character = my_character_int = getchar_timeout_us(5);
            if (my_character_int == ASCII_ENTER)
            {
                printf("INPUT MODE:");
                my_input_loop = true;
                while (my_input_loop)
                {
                    my_character = my_character_int = getchar_timeout_us(0);
                    if (my_character_int != PICO_ERROR_TIMEOUT)
                    {
                        if (my_character_int == ASCII_ENTER)
                        {
                            my_character_array[my_input_index] = '\0';
                            printf("\n");
                            if (!strcmp(my_character_array, "status"))
                            {
                                printf("STATUS...\n");
                            }
                            else if (!strcmp(my_character_array, "calib"))
                            {
                                printf("CALIBRATION...\n");
                            }
                            else if (!strcmp(my_character_array, "run"))
                            {
                                printf("RUN...\n");
                            }
                            // !!! Modify next not to accept with other symbol after number)
                            else if (!strncmp(my_character_array, "run ", strlen("run ")) && sscanf(my_character_array + strlen("run "), "%d", &my_number_of_runs))
                            {
                                printf("RUN...%d times\n", my_number_of_runs);
                            }
                            else
                            {
                                printf("INPUT ERROR! Available commands:'status', 'calib', 'run N'. (N - number)\n");
                            }
                            printf("\nPress ENTER for input mode.\n");
                            // reset_input_array(my_character_array, MY_STRING_LENGTH_MAX);
                            my_input_index = 0;
                            my_input_loop = false;
                        }

                        else if (my_character_int == ASCII_BACKSPACE && my_input_index > 0)
                        {
                            my_character_array[my_input_index] = ' ';
                            my_input_index--;
                            printf("\b \b");
                        }

                        else
                        {
                            if (my_character_int != ASCII_BACKSPACE && my_input_index < MY_STRING_LENGTH_MAX)
                            {
                                my_character_array[my_input_index] = my_character;
                                my_input_index++;
                                printf("%c", (char)my_character);
                            }
                        }
                    }
                }
            }
        }
    }
}
