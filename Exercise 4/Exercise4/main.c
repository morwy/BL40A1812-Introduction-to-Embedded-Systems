#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define LCD_MAX_STRING (32)

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