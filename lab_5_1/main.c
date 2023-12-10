#include "my_pico_definitions.h"

// For input
#define ASCII_SPACE 32
#define ASCII_ENTER 13
#define ASCII_BACKSPACE 8
#define ASCII_ESC 27
#define MY_STRING_LENGTH_MAX 10
#define NUMBER_OF_RUNS_DEF 8
#define CALIBRATION_ROUNDS 3
typedef enum
{
    STATE_IDLE = 1,
    STATE_INPUT = 2,
    STATE_CALIBRATION = 3,
    STATE_RUN = 4,
} State;

enum MotorState
{
    STATE_1,
    STATE_2,
    STATE_3,
    STATE_4,
    STATE_5,
    STATE_6,
    STATE_7,
    STATE_8
};

// Globals
bool my_input_toggle = false;
State program_state = STATE_IDLE;
enum MotorState current_motor_state = STATE_1;

// Interrupt handler for input mode
void my_interrupt_handler_input_mode()
{
    if (!my_input_toggle)
    {
        my_input_toggle = true;
        program_state = STATE_INPUT;
    }
}

void half_step_sequence()
{
    motor_step(1, 0, 0, 0);
    motor_step(1, 1, 0, 0);
    motor_step(0, 1, 0, 0);
    motor_step(0, 1, 1, 0);
    motor_step(0, 0, 1, 0);
    motor_step(0, 0, 1, 1);
    motor_step(0, 0, 0, 1);
    motor_step(1, 0, 0, 1);
}

void perform_step()
{
    switch (current_motor_state)
    {
    case STATE_1:
        motor_step(1, 0, 0, 0);
        current_motor_state = STATE_2;
        break;
    case STATE_2:
        motor_step(1, 1, 0, 0);
        current_motor_state = STATE_3;
        break;
    case STATE_3:
        motor_step(0, 1, 0, 0);
        current_motor_state = STATE_4;
        break;
    case STATE_4:
        motor_step(0, 1, 1, 0);
        current_motor_state = STATE_5;
        break;
    case STATE_5:
        motor_step(0, 0, 1, 0);
        current_motor_state = STATE_6;
        break;
    case STATE_6:
        motor_step(0, 0, 1, 1);
        current_motor_state = STATE_7;
        break;
    case STATE_7:
        motor_step(0, 0, 0, 1);
        current_motor_state = STATE_8;
        break;
    case STATE_8:
        motor_step(1, 0, 0, 1);
        current_motor_state = STATE_1;
        break;
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

    // Configuring motor, initialising pins
    my_configure_motor();

    // Variables
    char my_character;
    int my_character_int;
    char my_character_array[MY_STRING_LENGTH_MAX];
    int my_input_index = 0;
    int my_number_of_runs = NUMBER_OF_RUNS_DEF;
    int my_number_of_steps = -1;
    // bool opto_fork_reached_end = false;

    // Interrupts
    gpio_set_irq_enabled_with_callback(UART_RX_PIN, GPIO_IRQ_EDGE_RISE, true, &my_interrupt_handler_input_mode);

    // Printing first instructions
    printf("\nStart typing for input mode. Available commands: 'status', 'calib', 'run N'. (N - number)\n");

    // MAIN LOOP
    while (true)
    {
        switch (program_state)
        {
        case STATE_IDLE:
            // Cleaning input buffer
            clean_getchar_buffer();
            // Setting number of runs to default
            my_number_of_runs = NUMBER_OF_RUNS_DEF;
            // turning input interrupt  ON
            gpio_set_irq_enabled_with_callback(UART_RX_PIN, GPIO_IRQ_EDGE_RISE, true, &my_interrupt_handler_input_mode);
            while (program_state == STATE_IDLE)
            {
            }
            // turning input interrupt  OFF
            gpio_set_irq_enabled_with_callback(UART_RX_PIN, GPIO_IRQ_EDGE_RISE, false, &my_interrupt_handler_input_mode);
            break;
        case STATE_INPUT:
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
                            if (my_number_of_steps < 0)
                            {
                                printf("not available\n");
                            }
                            else
                            {
                                printf("Calibrated with %d steps per revolution.\n", my_number_of_steps);
                            }
                            program_state = STATE_IDLE;
                        }
                        else if (!strcmp(my_character_array, "calib"))
                        {
                            printf("CALIBRATION... (Input is not available)\n");
                            program_state = STATE_CALIBRATION;
                        }
                        else if (!strcmp(my_character_array, "run"))
                        {
                            printf("RUNNING %d * 1/8 REVOLUTIONS...\n", my_number_of_runs);
                            program_state = STATE_RUN;
                        }
                        // !!! Modify next to not accept with other symbol after number)
                        else if (!strncmp(my_character_array, "run ", strlen("run ")) && sscanf(my_character_array + strlen("run "), "%d", &my_number_of_runs))
                        {
                            printf("RUNNING %d * 1/8 REVOLUTIONS...\n", my_number_of_runs);
                            program_state = STATE_RUN;
                        }
                        else
                        {
                            printf("INPUT ERROR!\n");
                            program_state = STATE_IDLE;
                        }
                        my_input_index = 0;
                        my_input_toggle = false;
                    }

                    else if (my_character_int == ASCII_ESC)
                    {
                        printf("\nExited from input mode.\n");
                        my_input_index = 0;
                        my_input_toggle = false;
                        program_state = STATE_IDLE;
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
            break;
        case STATE_CALIBRATION:
            my_number_of_steps = 0;
            while (!gpio_get(PIN_OPTO_FORK))
            {
                perform_step();
            }
            while (gpio_get(PIN_OPTO_FORK))
            {
                perform_step();
            }
            // opto fork reached end starting to count until next end of it
            // Calibratinf CALIBRATON_ROUNDS times and findinf average
            for (int i = 0; i != CALIBRATION_ROUNDS; i++)
            {
                while (!gpio_get(PIN_OPTO_FORK))
                {
                    perform_step();
                    my_number_of_steps++;
                }
                while (gpio_get(PIN_OPTO_FORK))
                {
                    perform_step();
                    my_number_of_steps++;
                }
            }
            my_number_of_steps = my_number_of_steps / CALIBRATION_ROUNDS;
            program_state = STATE_IDLE;
            printf("Calibrated with %d steps per revolution.\n", my_number_of_steps);
            break;

        case STATE_RUN:
            if (my_number_of_steps > 0)
            {
                for (int i = 0; i != my_number_of_steps * my_number_of_runs / NUMBER_OF_RUNS_DEF; i++)
                {
                    perform_step();
                }
            }
            else
            {
                printf("ERROR! NOT CALIBRATED. CALIBRATE with 'calib' command");
            }
            program_state = STATE_IDLE;
            break;
        }
    }
}
