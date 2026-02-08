#include "uart.h"
#include "timer0.h"
#include "adc.h"

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
