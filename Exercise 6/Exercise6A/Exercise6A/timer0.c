#include <stdint.h>
#include "timer0.h"
#include "mcu.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include "critical.h"
#include "bit_ops.h"

/// Datasheet p. 126
#define TCCR0A_WGM_MODE_OFFSET (0)
#define TCCR0A_WGM_MODE_MASK (0b11)

/// Datasheet p. 128 Table 16-8
#define WGM_MODE_CLEAR_ON_COMPARE_MATCH (0b010)

/// Datasheet p. 129
#define TCCR0B_CLOCK_SELECT_OFFSET (0)
#define TCCRB0_CLOCL_SELECT_MASK (0b111)
#define TCCR0B_WGM_MODE_OFFSET (3)
#define TCCR0B_WGM_MODE_MASK (0b1)

/// Datasheet p. 130 Table 16-9
#define CLOCK_SELECT_CLKIO_DIV_1024 (0b101)

/// Datasheet p. 131
#define TIMSK_TIFR_TIMER_OVERFLOW_INTERRUPT_OFFSET (0)
#define TIMSK_TIFR_COMPARE_A_MATCH_INTERRUPT_OFFSET (1)

/// Human-readable registers
#define CONTROL_REGISTER_A (TCCR0A)
#define CONTROL_REGISTER_B (TCCR0B)
#define TIMER_COUNT (TCNT0)
#define OUTPUT_COMPARE_A (OCR0A)
#define OUTPUT_COMPARE_B (OCR0B)
#define INTERRUPT_MASK (TIMSK0)
#define INTERRUPT_FLAGS (TIFR0)

/// Compute timer trigger period
#define MILLISECONDS_PER_SECOND (1000)
#define CONFIGURED_PRESCALER (1024)
#define OCRA_PERIOD_10MS ((F_CPU * 10) / (MILLISECONDS_PER_SECOND) / (CONFIGURED_PRESCALER))

static uint32_t timer_ticks = 0;

ISR (TIMER0_COMPA_vect)
{
    timer_ticks++;
}

/// Set default value in all timer registers.
static void reset_timer0_module(void) {
    CONTROL_REGISTER_A = 0;
    CONTROL_REGISTER_B = 0;
    TIMER_COUNT = 0;
    OUTPUT_COMPARE_A = 0;
    OUTPUT_COMPARE_B = 0;
    INTERRUPT_MASK = 0;
    INTERRUPT_FLAGS = 0;
}

/// Configure timer mode which requires writing of 2 separate registers.
static void set_waveform_generation_mode(uint8_t mode) {
    SET_BITS(CONTROL_REGISTER_A, TCCR0A_WGM_MODE_OFFSET, mode & TCCR0A_WGM_MODE_MASK);
    SET_BITS(CONTROL_REGISTER_B, TCCR0B_WGM_MODE_OFFSET, (mode >> 2) & TCCR0B_WGM_MODE_MASK);
}

/// Set up the 8-bit timer/counter0.
/// This function provide precise intervals for triggering
/// ADC conversions and internal timing without busy waiting
void setup_timer0(void)
{
    reset_timer0_module();
    set_waveform_generation_mode(WGM_MODE_CLEAR_ON_COMPARE_MATCH);
    SET_BITS(CONTROL_REGISTER_B, TCCR0B_CLOCK_SELECT_OFFSET, CLOCK_SELECT_CLKIO_DIV_1024);
    OUTPUT_COMPARE_A = OCRA_PERIOD_10MS;
    SET_BITS(INTERRUPT_MASK, TIMSK_TIFR_COMPARE_A_MATCH_INTERRUPT_OFFSET, 1);
    TIMER_COUNT = 0;
    printf("CONTROL_REGISTER_A: %x\nCONTROL_REGISTER_B: %x\n", CONTROL_REGISTER_A, CONTROL_REGISTER_B);
}

/// Returns a copy of current wall timer value. Uses critical sections to ensure atomicity.
uint32_t get_time(void)
{
    enter_critical();
    uint32_t ticks = timer_ticks;
    exit_critical();
    return ticks;
}