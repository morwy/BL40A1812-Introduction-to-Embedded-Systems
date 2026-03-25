#ifndef TIMER0_H
#define TIMER0_H

#include <stdint.h>

/// Set up the 8-bit timer/counter0.
/// This function provide precise intervals for triggering
/// ADC conversions and internal timing without busy waiting
void setup_timer0(void);

/// Returns a copy of current wall timer value. Uses critical sections to ensure atomicity.
uint32_t get_time(void);

#endif //TIMER0_H