# c_lang_lab_1

Implement a program for switching LEDs on/off and dimming them. The program should work as follows:

SW1, the middle button is the on/off button. When button is pressed the state of LEDs is toggled.
Program must require the button to be released before the LEDs toggle again. Holding the button
may not cause LEDs to toggle multiple times.

SW0 and SW2 are used to control dimming when LEDs are in ON state. SW0 increases brightness
and SW2 decreases brightness. Holding a button makes the brightness to increase/decrease
smoothly. If LEDs are in OFF state the buttons have no effect.

When LED state is toggled to ON the program must use same brightness of the LEDs they were at
when they were switched off. If LEDs were dimmed to 0% then toggling them on will set 50%
brightness.

PWM frequency divider must be configured to output 1 MHz frequency and PWM frequency must
be 1 kHz.

How to configure RP2040 PWM
• Call pwm_gpio_to_slice_num and pwm_gpio_to_channel to get slice and
channel of your GPIO pin
• Call pwm_set_enabled to stop PWM
• Call pwm_get_default_config to default PWM configuration
• Call pwm_config_set_clkdiv_int to set clock divider
• Call pwm_config_set_wrap on the default config to set wrap (TOP)
• Call pwm_init with your slice number and the config you created with start
set to false
• Call pwm_set_chan_level with your slice and channel numbers to set level
(CC) → duty cycle
• Call gpio_set_function to select PWM mode for your pin
• Call pwm_set_enabled to start PWM

• Adjust duty cycle as needed by calling pwm_set_chan_level

