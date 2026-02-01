#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "lcd.h"

#define LCD_MAX_STRING (32)

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

/**
 * Protected write to LCD that checks that provided pointer is a valid
 * null-terminated string.
 * @param string Pointer to the string that should be printed
 */
static void write_to_lcd(const char *string)
{
	uint8_t len = strnlen(string, LCD_MAX_STRING);
	if (LCD_MAX_STRING == len)
	{
		printf("Failed to print LCD string. Too big or lacks NULL-terminator.\r\n");
		// Since we have null-terminator we print the bad string one character at a time.
		for (uint8_t i = 0; i < len; i++)
		{
			printf("%c", string[i]);
		}
		printf("\r\n");
		handle_error(1);
	}
	else
	{
		printf("LCD output: '%s'\r\n", string);
		lcd_puts(string);
	}
}

static void setup(void)
{
	// Initialize serial port for standard library.
	printf("Initializing serial port.\r\n");
	uint8_t rc = setup_uart_io();
	handle_error(rc);
	printf("Serial port initialized.\r\n");

	// Initialize keypad.
	printf("Initializing keypad.\r\n");
	rc = KEYPAD_Init();
	handle_error(rc);
	printf("Keypad initialized.\r\n");

	// Initialize LCD.
	printf("Initializing LCD.\r\n");
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	printf("LCD initialized.\r\n");

	write_to_lcd("Ready");
}

int main(void)
{
	static char key_str[32];
	setup();
	uint32_t memory = 0;
	while (1)
	{
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
	}
}