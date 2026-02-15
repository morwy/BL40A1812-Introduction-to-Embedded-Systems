#include "timer1.h"
#include "mcu.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include "bit_ops.h"
#include <stdint.h>

/// Datasheet p. 154
#define TCCR1A_WGM_MODE_OFFSET (0)
#define TCCR1A_WGM_MODE_MASK (0b11)
#define TCCR1A_COMPARE_MODE_CH_A_OFFSET (6)
#define TCCR1A_COMPARE_MODE_CH_A_MASK (0b11)
#define TCCR1A_COMPARE_MODE_NORMAL_IO (0b00)
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

/// Set default value in all timer registers.
static void reset_timer1_module(void) {
    CONTROL_REGISTER_A = 0;
    CONTROL_REGISTER_B = 0;
    TIMER_COUNT = 0;
    OUTPUT_COMPARE_A = 0;
    OUTPUT_COMPARE_B = 0;
    INTERRUPT_MASK = 0;
    INTERRUPT_FLAGS = 0;
}

// Configure timer mode which requires writing of 2 separate registers.
static void set_waveform_generation_mode(uint8_t mode) {
    SET_BITS(CONTROL_REGISTER_A, TCCR1A_WGM_MODE_OFFSET, mode & TCCR1A_WGM_MODE_MASK);
    SET_BITS(CONTROL_REGISTER_B, TCCR1B_WGM_MODE_OFFSET, (mode >> 2) & TCCR1B_WGM_MODE_MASK);
}

/// Set up the 8-bit timer/counter1.
/// This function configures timer1 to generate PWM of fixed
/// 50% duty cycle and varying frequency.
void setup_timer1(void) {
    reset_timer1_module();
    set_waveform_generation_mode(WGM_MODE_PWM_PHASE_FREQ_CORRECT_OCRA);
    timer1_channel_A_off();

    printf("Control register A: %x\r\n", CONTROL_REGISTER_A);
    printf("Control register B: %x\r\n", CONTROL_REGISTER_B);
    printf("Timer count: %x\r\n", TIMER_COUNT);
    printf("Output compare A: %x\r\n", OUTPUT_COMPARE_A);
    printf("Output compare B: %x\r\n", OUTPUT_COMPARE_B);
    printf("Interrupt mask: %x\r\n", INTERRUPT_MASK);
    printf("Interrupt flags: %x\r\n", INTERRUPT_FLAGS);
}

/// Disables PWM output on channel A pin
void timer1_channel_A_off(void) {
    CLEAR_BITS(CONTROL_REGISTER_A, TCCR1A_COMPARE_MODE_CH_A_OFFSET, TCCR1A_COMPARE_MODE_CH_A_MASK);
    SET_BITS(CONTROL_REGISTER_A, TCCR1A_COMPARE_MODE_CH_A_OFFSET, TCCR1A_COMPARE_MODE_NORMAL_IO);
}

/// Enables PWM output on channel A pin
void timer1_channel_A_on(void) {
    CLEAR_BITS(CONTROL_REGISTER_A, TCCR1A_COMPARE_MODE_CH_A_OFFSET, TCCR1A_COMPARE_MODE_CH_A_MASK);
    SET_BITS(CONTROL_REGISTER_A, TCCR1A_COMPARE_MODE_CH_A_OFFSET, TCCR1A_COMPARE_MODE_TOGGLE_ON_MATCH);
}

/// Changes prescaler and conparison register
/// to produce the desired frequency of PWM signal
void timer1_set_frequency(uint16_t frequency_hz) {
    timer1_prescaler_t prescaler;

    if (frequency_hz >= 64) {
        prescaler = TIMER1_PRESCALER_1;
    } 
    else if (frequency_hz >= 8) {
        prescaler = TIMER1_PRESCALER_8;
    } 
    else {
        prescaler = TIMER1_PRESCALER_64;
    }

    const uint16_t divider = PRESCALER_DIVIDERS[prescaler];
    /// Extra division by 4 comes from 2 factors:
    /// 1) In phase and frequency correct PWM mode the counter counts up to OCRA and then back to 0 so it dou
    /// the effective period to 2 * OCRA
    /// 2) The output is toggled once every time the timer reaches OCRA. Therefore it takes two full counting
    /// toggle low to high and then high to low.
    const uint32_t ticks_per_s = ((uint32_t) F_CPU) / divider / 4;
    
    OUTPUT_COMPARE_A = ticks_per_s / frequency_hz;

    // Clear previous prescaler bits and set new ones
    CLEAR_BITS(CONTROL_REGISTER_B, TCCR1B_CLOCK_SELECT_OFFSET, TCCR1B_CLOCK_SELECT_MASK);
    SET_BITS(CONTROL_REGISTER_B, TCCR1B_CLOCK_SELECT_OFFSET, prescaler);
}

