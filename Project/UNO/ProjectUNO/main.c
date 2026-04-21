/*
 * Mega_UART.c
 * Source : https://appelsiini.net/2011/simple-usart-with-avr-libc/
 *          https://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
 *          https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf 
 */ 

#include "mcu.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "uart.h"
#include "bit_ops.h"
#include "board_config.h"
#include "pin_config.h"

#define SLAVE_ADDRESS 0b1010111 // 87 as decimal. This address must be the same as master address.

// Start and stop commands
#define UNO_CMD_OBSTACLE_START (0x01) // start blinking + buzzer/melody
#define UNO_CMD_OBSTACLE_STOP  (0x02) // stop buzzer/melody

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

	// Configure TWI as slave
	TWCR |= (1 << TWEA) | (1 << TWEN);
	TWCR &= ~((1 << TWSTA) | (1 << TWSTO));
	TWAR = (SLAVE_ADDRESS << 1); // address 

	CLEAR_BIT(LED_13_PORT, LED_13_PIN);    // Default output value LOW
    SET_BIT(LED_13_DIRECTION, LED_13_PIN); // Set pin to OUTPUT

	uint8_t obstacle_flag = 0; // 0 = no obstacle, 1 = obstacle detected
	uint8_t twi_status = 0;
	uint8_t buzzer_enabled = 0;
	uint8_t blink_enabled = 0;

	// Testing LED 1s ON then off
	SET_BIT(LED_13_PORT, LED_13_PIN);
	_delay_ms(10000);
	CLEAR_BIT(LED_13_PORT, LED_13_PIN);
	printf("hello delay done\r\n");

	char test_char_array[16]; // 16-bit array, assumes that the int given is 16-bits

    while (1) 
    {
		printf("hello\r\n");
		// Wait until TWI interrupt flag is set
		while (!(TWCR & (1 << TWINT)))
		{;}

		twi_status = (TWSR & 0xF8);
		itoa(twi_status, test_char_array, 16);
		printf(test_char_array);
		printf(" ");

		/* // Clear TWINT and keep ACK enabled so we remain addressable
		TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

		printf("hello2\r\n");
		while (!(TWCR & (1 << TWINT)))
		{;}

		twi_status = (TWSR & 0xF8);
		itoa(twi_status, test_char_array, 16);
		printf(test_char_array);
		printf(" "); */

		// Slave transmitter mode (master reads)
		if ((twi_status == 0xA8) || (twi_status == 0xB8))
		{
			// SLA+R received OR previous byte sent and ACK received.
			// Read the obstacle flag from the sensor and provide the current obstacle flag byte.
			//
			// TODO (obstacle sensor): update obstacle_flag based on your sensor here.
			// - obstacle_flag = 0; // no obstacle
			// - obstacle_flag = 1; // obstacle detected
			TWDR = obstacle_flag;
			// Clear TWINT to transmit the obstacle flag
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
		}
		// Slave receiver mode (master writes command)
		else if ((twi_status == 0x60) || (twi_status == 0x68))
		{
			// Own SLA+W received. Next state will be data receive.
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
		}
		else if ((twi_status == 0x80) || (twi_status == 0x90))
		{
			// Data received, ACK returned
			uint8_t cmd = TWDR;
			if (cmd == UNO_CMD_OBSTACLE_START)
			{
				// Buzzer sound on and blink LED
				blink_enabled = 1;
				buzzer_enabled = 1;
			}
			else if (cmd == UNO_CMD_OBSTACLE_STOP)
			{
				// Stop buzzer sound
				buzzer_enabled = 0;
				// for testing
				blink_enabled = 0;
			}

			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
		}
		else if ((twi_status == 0xC0) || (twi_status == 0xC8) || (twi_status == 0xA0))
		{
			// Transmission ended (NACK / last byte / STOP). Clear interrupt flag
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
		}
		else
		{
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
		}
		

		// TODO (UNO LEDs): if blink_enabled==1, blink obstacle LED 3 times, then blink_enabled=0
		// Testing (no blinking)
		if (blink_enabled == 1)
		{
			SET_BIT(LED_13_PORT, LED_13_PIN);
		}
		else if (blink_enabled == 0)
		{
			CLEAR_BIT(LED_13_PORT, LED_13_PIN);
		}

		// TODO (UNO buzzer): if buzzer_enabled==1, play melody (>=5 notes) here; stop when buzzer_enabled==0

		
		printf("UNO obstacle=%u blink=%u buzzer=%u\r\n", obstacle_flag, blink_enabled, buzzer_enabled);
    }
 
    return 0;
}
