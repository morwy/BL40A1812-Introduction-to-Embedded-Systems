#include "uart.h"
#include "timer0.h"
#include "adc.h"
#include "lcd.h"

#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

/// There is not much we can do for now. This function will be improved in future.
static void handle_error(uint8_t return_code)
{
	// Non-zero return code indicates critical fault
	if (return_code)
	{
		while (1)
			;
	}
}

static void setup(void)
{
	// Initialize serial port for standard library
	printf("Setting up the serial port...\r\n");
	uint8_t rc = setup_uart_io();
	handle_error(rc);

	// Set Port C pin 3 as input without pull-up
	DDRC &= ~(1 << DDC3);	 // Clear bit 3 to set as input
	PORTC &= ~(1 << PORTC3); // Clear bit 3 to disable pull-up

	// Call setup function of the Timer 0
	printf("Setting up Timer 0...\r\n");
	setup_timer0();

	// Call setup function of the ADC
	printf("Setting up ADC...\r\n");
	setup_adc();

	// Configure sleep mode of the processor set_sleep_mode(SLEEP_MODE_IDLE);
	printf("Configuring sleep mode...\r\n");
	set_sleep_mode(SLEEP_MODE_IDLE);

	// Enable interrupts globally with sei();
	printf("Enabling global interrupts...\r\n");
	sei();

	// Initialize LCD by calling lcd_init(LCD_DISP_ON);
	printf("Initializing LCD...\r\n");
	lcd_init(LCD_DISP_ON);
	printf("LCD initialized.\r\n");

	// Clear LCD screen with lcd_clrscr();
	printf("Clearing LCD screen...\r\n");
	lcd_clrscr();
	printf("LCD cleared.\r\n");

	// Write ’Ready’ to LCD with write_to_lcd implemented before
	write_to_lcd("Ready");

	// Busy-wait 1 second
	printf("Waiting for 1 second...\r\n");
	_delay_ms(1000);

	printf("Done.\r\n");
}

int main(void)
{
	setup();

	uint16_t adc_result = 0;
	uint32_t previous_time = 0;
	uint32_t new_time = 0;

	while (1)
	{
		/// Try getting the latest ADC result with try_reading_adc interface
		if (try_reading_adc(&adc_result))
		{
			/// If value obtained format it to a buffer string with snprintf and write to LCD: (line 1) "The ADC result", (line 2) the measured value.
			char buffer[16];
			snprintf(buffer, sizeof(buffer), "%u", adc_result);
			write_to_lcd("The ADC result");
			write_to_lcd(buffer);
		}

		/// Obtain update counter by reading get_time() interface and right shifting it by 6 bits (equivalent to division by 64). This will change value every 10*64 -> 640 ms.
		new_time = get_time() >> 6;

		/// Loop in while until the value of update counter changes. On every iteration call sleep_mode() to cause processor to go into energy-conservation mode. Poll the time after this call to update the exit condition once the CPU is woken up by some interrupt.
		while (new_time == previous_time)
		{
			sleep_mode();
			new_time = get_time() >> 6;
		}

		previous_time = new_time;
	}
}