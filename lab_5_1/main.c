#include "my_pico_definitions.h"

// I2C Configuration
#define MY_I2C_BAUD_RATE 100000
#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

// UART
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// For input
#define ASCII_SPACE 32
#define ASCII_ENTER 13
#define ASCII_BACKSPACE 8
#define ASCII_ESC 27
#define MY_STRING_LENGTH_MAX 10

// #define STRING_STATUS "status"
// #define STRING_CALIB "calib"
// #define STRING_RUN "run "

#define MOTOR_STEP_DELAY_US 5000
void full_step_sequence();
void motor_steps_full(int steps);
void motor_step(int in1, int in2, int in3, int in4);

// Interrupt handler for input mode
bool my_input_toggle = false;
void my_interrupt_handler_input_mode()
{
    if (!my_input_toggle)
    {
        my_input_toggle = true;
    }
}

void full_step_sequence()
{
    motor_step(1, 0, 0, 0);
    motor_step(0, 1, 0, 0);
    motor_step(0, 0, 1, 0);
    motor_step(0, 0, 0, 1);
}

void motor_steps_full(int steps)
{
    for (int i = 0; i != steps; i++)
    {
        full_step_sequence();
    }
}

void motor_step(int in1, int in2, int in3, int in4)
{

    gpio_put(PIN_MOTOR_1, in1);
    gpio_put(PIN_MOTOR_2, in2);
    gpio_put(PIN_MOTOR_3, in3);
    gpio_put(PIN_MOTOR_4, in4);
    busy_wait_us(MOTOR_STEP_DELAY_US);
}

void motor_turn_off_coils()
{

    gpio_put(PIN_MOTOR_1, 0);
    gpio_put(PIN_MOTOR_2, 0);
    gpio_put(PIN_MOTOR_3, 0);
    gpio_put(PIN_MOTOR_4, 0);
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

    // Configure MOTOR
    gpio_init(PIN_OPTO_FORK);
    gpio_set_dir(PIN_OPTO_FORK, GPIO_IN);
    gpio_pull_up(PIN_OPTO_FORK);

    gpio_init(PIN_PIEZO_SENSOR);
    gpio_set_dir(PIN_PIEZO_SENSOR, GPIO_IN);
    gpio_pull_up(PIN_PIEZO_SENSOR);

    gpio_init(PIN_MOTOR_1);
    gpio_set_dir(PIN_MOTOR_1, GPIO_OUT);

    gpio_init(PIN_MOTOR_2);
    gpio_set_dir(PIN_MOTOR_2, GPIO_OUT);

    gpio_init(PIN_MOTOR_3);
    gpio_set_dir(PIN_MOTOR_3, GPIO_OUT);

    gpio_init(PIN_MOTOR_4);
    gpio_set_dir(PIN_MOTOR_4, GPIO_OUT);

    // Variables
    char my_character;
    int my_character_int;
    char my_character_array[MY_STRING_LENGTH_MAX];
    int my_input_index = 0;
    int my_number_of_runs = 0;

    // Interrupts
    gpio_set_irq_enabled_with_callback(UART_RX_PIN, GPIO_IRQ_EDGE_RISE, true, &my_interrupt_handler_input_mode);

    // Printing first instructions
    printf("\nStart typing for input mode. Available commands: 'status', 'calib', 'run N'. (N - number)\n");

    // MAIN LOOP
    while (true)
    {
        // !! Implement state machine
        // Input mode toggled by interrupt
        while (my_input_toggle)
        {
            my_character = my_character_int = getchar_timeout_us(5);
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
                        motor_steps_full(my_number_of_runs);
                        motor_turn_off_coils();
                    }
                    else
                    {
                        printf("INPUT ERROR!\n");
                    }
                    printf("\nStart typing for input mode. Available commands: 'status', 'calib', 'run N'. (N - number)\n");
                    my_input_index = 0;
                    my_input_toggle = false;
                }

                else if (my_character_int == ASCII_ESC)
                {
                    printf("\nExited from input mode.\n");
                    printf("\nStart typing for input mode. Available commands: 'status', 'calib', 'run N'. (N - number)\n");
                    my_input_index = 0;
                    my_input_toggle = false;
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
