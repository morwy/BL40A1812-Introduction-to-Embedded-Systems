#include "timer1.h"
#include "mcu.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#include "critical.h"
#include "bit_ops.h"

/// Datasheet p. 154
#define TCCR1A_WGM_MODE_OFFSET (0)
#define TCCR1A_WGM_MODE_MASK (0b11)
#define TCCR1A_COMPARE_MODE_CH_A_OFFSET (6)
#define TCCR1A_COMPARE_MODE_CH_A_MASK (0b11)

#define TCCR1A_COMPARE_MODE_NORMAL_IO (0)
#define TCCR1A_COMPARE_MODE_TOGGLE_ON_MATCH (0b01)

/// Datasheet p. 145 Table 17-2
#define WGM_MODE_PWM_PHASE_FREQ_CORRECT_OCRA (9)

/// Datasheet p. 156
#define TCCR1B_CLOCK_SELECT_OFFSET (0)
#define TCCR1B_CLOCK_SELECT_MASK (0b111)
#define TCCR1B_WGM_MODE_OFFSET (3)
#define TCCR1B_WGM_MODE_MASK (0b11)

/// Human-readable registers
#define CONTROL_REGISTER_A (TCCR1A)
#define CONTROL_REGISTER_B (TCCR1B)
#define TIMER_COUNT (TCNT1)
#define OUTPUT_COMPARE_A (OCR1A)
#define OUTPUT_COMPARE_B (OCR1B)
#define INTERRUPT_MASK (TIMSK1)
#define INTERRUPT_FLAGS (TIFR1)

typedef enum timer1_prescaler {
    TIMER1_CLOCK_OFF = 0,
    TIMER1_PRESCALER_1 = 1,
    TIMER1_PRESCALER_8 = 2,
    TIMER1_PRESCALER_64 = 3,
    TIMER1_PRESCALER_256 = 4,
    TIMER1_PRESCALER_1024 = 5,
    TIMER1_EXTERNAL_CLOCK_FALLING = 6,
    TIMER1_EXTERNAL_CLOCK_RAISING = 7,
} timer1_prescaler_t;

const uint16_t PRESCALER_DIVIDERS[] = {1, 1, 8, 64, 256, 1024, 1, 1};
    
static void reset_timer1_module(void)
{
    CONTROL_REGISTER_A = 0;
    CONTROL_REGISTER_B = 0;
    TIMER_COUNT = 0;
    OUTPUT_COMPARE_A = 0;
    OUTPUT_COMPARE_B = 0;
    INTERRUPT_MASK = 0;
    INTERRUPT_FLAGS = 0;
}

static void set_waveform_generation_mode(uint8_t mode) {
     SET_BITS(CONTROL_REGISTER_A, TCCR1A_WGM_MODE_OFFSET, mode & TCCR1A_WGM_MODE_MASK);
     SET_BITS(CONTROL_REGISTER_B, TCCR1B_WGM_MODE_OFFSET, (mode >> 2) & TCCR1B_WGM_MODE_MASK);
}

void timer1_channel_A_off(void) {
    CLEAR_BITS(CONTROL_REGISTER_A, TCCR1A_COMPARE_MODE_CH_A_OFFSET, TCCR1A_COMPARE_MODE_CH_A_MASK);
    SET_BITS(CONTROL_REGISTER_A, TCCR1A_COMPARE_MODE_CH_A_OFFSET, TCCR1A_COMPARE_MODE_NORMAL_IO);
}

void timer1_channel_A_on(void) {
    CLEAR_BITS(CONTROL_REGISTER_A, TCCR1A_COMPARE_MODE_CH_A_OFFSET, TCCR1A_COMPARE_MODE_CH_A_MASK);
    SET_BITS(CONTROL_REGISTER_A, TCCR1A_COMPARE_MODE_CH_A_OFFSET, TCCR1A_COMPARE_MODE_TOGGLE_ON_MATCH);
}

void timer1_set_frequency(uint16_t frequency) {
    timer1_prescaler_t prescaler;
    
    if (frequency < 8) {
        prescaler = TIMER1_PRESCALER_64;
    } if (frequency < 64) {
        prescaler = TIMER1_PRESCALER_8;
    } else {
        prescaler = TIMER1_PRESCALER_1;
    }

    SET_BITS(CONTROL_REGISTER_B, TCCR1B_CLOCK_SELECT_OFFSET, TCCR1B_CLOCK_SELECT_MASK);
    SET_BITS(CONTROL_REGISTER_B, TCCR1B_CLOCK_SELECT_OFFSET, prescaler);

    const uint16_t divider = PRESCALER_DIVIDERS[prescaler];
    
    const uint32_t ticks_per_s = ((uint32_t) F_CPU) / divider / 4;

    OUTPUT_COMPARE_A = ticks_per_s / frequency;

}


void setup_timer1(void)
{
    reset_timer1_module();
    set_waveform_generation_mode(WGM_MODE_PWM_PHASE_FREQ_CORRECT_OCRA);
    timer1_channel_A_off();
}