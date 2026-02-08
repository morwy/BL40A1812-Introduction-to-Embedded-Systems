#include "adc.h"
#include "critical.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include "bit_ops.h"

/// Human-readable names
#define ADC_MULTIPLEXER (ADMUX)
#define ADC_CONTROL_STATUS_A (ADCSRA)
#define ADC_CONTROL_STATUS_B (ADCSRB)

/// Datasheet p. 281
#define ADMUX_MUX_OFFSET (0)
#define ADMUX_MUX_MASK (0b11111)
// #define ADMUX_MUX_SINGLE_A3 ()
#define ADMUX_REFERENCE_SELECT_OFFSET (6)
#define ADMUX_REFERENCE_SELECT_MASK (0b11)
#define ADMUX_REFERENCE_SELECT_AVCC (0b01)

/// Datasheet p. 285
#define ADCSRA_CLOCK_PRESCALER_OFFSET (0)
#define ADCSRA_CLOCK_PRESCALER_MASK (0b111)
#define ADCSRA_CLOCK_PRESCALER_DIV_32 (0b101)
#define ADCSRA_ADC_INTERRUPT_ENABLE_OFFSET (3)
#define ADCSRA_AUTO_TRIGGER_OFFSET (5)
#define ADCSRA_ADC_ENABLE_OFFSET (7)

#define ADCSRB_TRIGGER_OFFSET (0)
#define ADCSRB_TRIGGER_MASK (0b111)

#define ADCSRB_TRIGGER_TIMER0_COMPARE_MATCH_A (0b011)

#define ADCSRB_MUX_OFFSET (3)
#define ADCSRB_MUX_MASK (0b1)

static uint16_t latest_adc_value = 0;  ///< to hold the latest ADC conversion result
static bool adc_conv_complete = false; ///< a flag that indicates ADC conversion is completed

/* When ADC conversion is completed, the result will be read and set it to true */
ISR(ADC_vect)
{
	latest_adc_value = ADC;
	adc_conv_complete = true;
}

/**
 * This function is used to poll the latest ADC conversion result.
 * @param uint16_t * output Pointer to location where to store the result of conversion
 *
 * return bool True if the result was ready and written to value pointed by result, false otherwise
 * */
bool try_reading_adc(uint16_t *output)
{
	enter_critical_section();

	bool result = false;

	if (adc_conv_complete)
	{
		*output = latest_adc_value;
		adc_conv_complete = false;
		result = true;
	}

	exit_critical_section();

	return result;
}

/// Write selected multiplexer mode to bits of ADC_MULTIPLEXER and ADC_CONTROL_STATUS_B
static void select_input_channel(uint8_t adc_mux)
{
	SET_BITS(ADC_MULTIPLEXER, ADMUX_MUX_OFFSET, adc_mux & ADMUX_MUX_MASK);
	SET_BITS(ADC_CONTROL_STATUS_B, ADCSRB_MUX_OFFSET, (adc_mux >> 4) & ADCSRB_MUX_MASK);
}

static void reset_adc_module(void)
{
	latest_adc_value = 0;
	adc_conv_complete = false;
	/// WTF goes here
}

/**
 * Set up the ADC conversion.
 * These functions converts input voltage on A3 into 10 bit digital value (0-1023)
 **/
void setup_adc(void)
{
	/// Reset the module
	reset_adc_module();

	/// Select ADC reference voltage (Avcc)
	SET_BITS(ADC_MULTIPLEXER, ADMUX_REFERENCE_SELECT_OFFSET, ADMUX_REFERENCE_SELECT_AVCC);

	/// Enable internal prescaler (divided by 32) for ADC sampling clock
	SET_BITS(ADC_CONTROL_STATUS_A, ADCSRA_CLOCK_PRESCALER_OFFSET, ADCSRA_CLOCK_PRESCALER_DIV_32);

	/// Link the ADC auto trigger to Timer/Counter0 compare match A
	SET_BITS(ADC_CONTROL_STATUS_B, ADCSRB_TRIGGER_OFFSET, ADCSRB_TRIGGER_TIMER0_COMPARE_MATCH_A);

	/// Enable ADC interrupt
	SET_BIT(ADC_CONTROL_STATUS_A, ADCSRA_ADC_INTERRUPT_ENABLE_OFFSET);

	/// Enable Auto Trigger mode
	SET_BIT(ADC_CONTROL_STATUS_A, ADCSRA_AUTO_TRIGGER_OFFSET);

	/// Enable ADC
	SET_BIT(ADC_CONTROL_STATUS_A, ADCSRA_ADC_ENABLE_OFFSET);

	/// Log values of ADC registers for debugging
	printf("ADC_MULTIPLEXER: %x\n", ADC_MULTIPLEXER);
	printf("ADC_CONTROL_STATUS_A: %x\n", ADC_CONTROL_STATUS_A);
	printf("ADC_CONTROL_STATUS_B: %x\n", ADC_CONTROL_STATUS_B);
}