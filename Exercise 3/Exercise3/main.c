/*
 * Mega_UART.c
 * Source : https://appelsiini.net/2011/simple-usart-with-avr-libc/
 *          https://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
 *          https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf 
 */ 

#include "mcu.h"

#include <util/delay.h>
#include <stdio.h>

#include "uart.h"

/// There is not much we can do for now. This function will be improved in future.
static void handle_error(uint8_t return_code)
{
	// Non-zero return code indicates critical fault
	if (return_code)
	{
	    while(1);
	}
}

int main(void)
{
    uint8_t rc = setup_uart_io();
    handle_error(rc);
    char username[32] = {0};

    // print out using the UART. This can be accessed via terminals such as PuTTY.
    printf("Hello World! What is your name (max 30 characters)?\r\n");

    // Read username safely
    fgets(username, sizeof(username), stdin);

    for (size_t i = 0; i < sizeof(username); i++)
    {
		if (username[i] == 10) // if it is an LF
		{
			username[i] = 0; // finished the string
			break;
		}
    }

    while (1) 
    {
        printf("Hello, %s!\r\n", username);
        _delay_ms(1000);
    }
 
    return 0;
}
