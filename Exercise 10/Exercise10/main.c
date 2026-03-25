#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>

#define FOSC 16000000UL // system clock frequency
#define BAUD 9600 // baud rate
#define MYUBRR (FOSC/16/BAUD-1) //Define the UBBR value using the equation in datasheet p.173 Table 20-1
#define SLAVE_ADDRESS 0b1010111 // 87 as decimal. You can try another address but ensure that master and slave have the same address.

static void USART_init(uint16_t ubrr)
{
	/* Set baud rate in the USART Baud Rate Registers (UBRR) */
	UBRR0H = (unsigned char)(ubrr >> 8); // see example datasheet p. 176
	UBRR0L = (unsigned char)ubrr;

	/* Enable receiver and transmitter on RX0 and TX0 */
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0); // see example datasheet p. 176

	/* Set frame format: 8 bit data, 2 stop bit */
	UCSR0C |= (1 << USBS0) | (3 << UCSZ00); // see example datasheet p. 176
}

static void USART_Transmit(unsigned char data, FILE *stream)
{
	/* Wait until the transmit buffer is empty*/
	while (!(UCSR0A & (1 << UDRE0))) // see example datasheet p.177
	{
		;
	}

	/* Puts the data into a buffer, then sends/transmits the data */
	UDR0 = data;
}

static char USART_Receive(FILE *stream)
{
	/* Wait until the transmit buffer is empty*/
	while (!(UCSR0A & (1 << RXC0))) // see example datasheet p. 180
	{
		;
	}

	/* Get the received data from the buffer */
	return UDR0;
}

// Setup the stream functions for UART, read https://appelsiini.net/2011/simple-usart-with-avr-libc/
FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);

int main(void)
{
	// initialize the UART with 9600 BAUD
	USART_init(MYUBRR);

	// redirect the stdin and stdout to UART functions,  read  https://appelsiini.net/2011/simple-usart-with-avr-libc/
	stdout = &uart_output;
	stdin = &uart_input;

	TWBR = 0x03; // TWI bit rate register.
	TWSR = 0x00; // TWI status register prescaler value set to 1
	
	TWCR |= (1 << TWEN); // The TWEN must be set to enable the TWI, datasheet p. 248
	
	unsigned char twi_send_data[20] = "master to slave\n"; //Instead of SPI, now is TWI
	char test_char_array[16]; // 16-bit array, assumes that the int given is 16-bits
	uint8_t twi_status = 0;
	
	while(true)
	{
		TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
		
		while (!(TWCR & (1 << TWINT)))
		{;}
			
		twi_status = (TWSR & 0xF8);
		
		twi_status = (TWSR & 0xF8);
		itoa(twi_status, test_char_array, 16);
		printf(test_char_array);
		printf(" ");
		
		TWDR = 0b10101110; //SLA_W --> 7-bit slave address and 1 write bit//p. 247, p. 263
		// Slave address + write bit '0' as an LSB
		
		TWCR = (1 << TWINT) | (1 << TWEN);
		
		while (!(TWCR & ( 1<< TWINT)))
		{;}
			
		twi_status = (TWSR & 0xF8);
		itoa(twi_status, test_char_array, 16);
		printf(test_char_array);
		printf(" ");
		
		for(int8_t twi_data_index = 0; twi_data_index < sizeof(twi_send_data); twi_data_index++)
		{
			// Load data into TWDR
			TWDR = twi_send_data[twi_data_index];
			// Clear TWINT bit in the TWCR to start transmission of data
			TWCR = (1 << TWINT) | (1 << TWEN);
			
			while (!(TWCR & ( 1<< TWINT)))
			{;}
				
			twi_status = (TWSR & 0xF8);
			itoa(twi_status, test_char_array, 16);
			printf(test_char_array);
			printf(" ");
		} //this is end of for command in the step 5b.
		
		TWCR = (1 << TWINT) | (1 << TWEN) |(1 << TWSTO);
		printf("\n");
		
		_delay_ms(1000);
	}
	
	return EXIT_SUCCESS;
}