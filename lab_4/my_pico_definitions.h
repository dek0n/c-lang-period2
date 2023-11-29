#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"

// Pins
#define PIN_LED1 22
#define PIN_LED2 21
#define PIN_LED3 20

#define PIN_SW0 9
#define PIN_SW1 8
#define PIN_SW2 7

#define PIN_ROTA 10
#define PIN_ROTB 11
#define PIN_ROT_Push 12

// For PWM
#define PWM_CHANNEL_TOP 999         // +1 Determines frequency?
#define PWM_CHANNEL_COMPARE_LEVEL 0 // Initial compare level of a channel (determines initial brightness)
#define PWM_CLOCK_DIVIDER 125       // Initial PWM 125Mhz/125 will give 1MHz

// Other
#define MAX_LED_BRIGHTNESS (PWM_CHANNEL_TOP + 1) // max compare level of the PIN's PWM from 0 to PWM_COUNTER_WRAP

struct SwitchStates // structure for sw buttons states
{
    int switch0;
    int switch1;
    int switch2;

    bool sw_changed;
    
    bool sw_all_toggle;

    int rot_push;
    int rot_turn_direction; // 0 counterclock, 1 clockwise
    bool rot_turn_toggle;
};

typedef struct LedStates // structure for led states
{
    uint8_t state;
    uint8_t not_state;
} ledstate;

// Function initializations
void initialize_pwm_pin(int pin_for_pwm); // maybe change name
bool rt_callback_function_sw(struct repeating_timer *rt);
void initialize_all_leds();
void initialize_all_sw_buttons();
void all_leds_on(int led_brightness);
void all_leds_off();
void process_DevEui(const char *devEuiWithPrefix, char *result);
void clear_uart_buffer(uart_inst_t *uart);
void print_time_stamp_s();
void print_led_states(ledstate *ls);
bool led_state_is_valid(ledstate *ls);
void set_led_state(ledstate *ls, uint8_t value);
void print_binary(uint8_t value);
void update_leds_from_led_states(ledstate *ls);

// void my_interrupt_handler(); // this function is currently in main.c

// GPIO        00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
// PWM Channel 0A 0B 1A 1B 2A 2B 3A 3B 4A 4B 5A 5B 6A 6B 7A 7B , A = channel 0, B = channel 1

// GPIO        15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30
// PWM Channel 0A 0B 1A 1B 2A 2B 3A 3B 4A 4B 5A 5B 6A 6B 7A 7B , A = channel 0, B = channel 1
