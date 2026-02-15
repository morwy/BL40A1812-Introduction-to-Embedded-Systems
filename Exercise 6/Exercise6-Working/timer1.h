#ifndef TIMER1_H
#define TIMER1_H
#include <stdint.h>

/// Set up the 8-bit timer/counter1.
/// This function configures timer1 to generate PWM of fixed
/// 50% duty cycle and varying frequency.
void setup_timer1(void);

/// Disables PWM output on channel A pin
void timer1_channel_A_off(void);

/// Enables PWM output on channel A pin
void timer1_channel_A_on(void);

/// Changes prescaler and conparison register
/// to produce the desired frequency of PWM signal
void timer1_set_frequency(uint16_t frequency_hz);

#endif //TIMER1_H