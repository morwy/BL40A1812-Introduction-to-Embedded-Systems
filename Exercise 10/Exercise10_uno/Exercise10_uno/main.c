#define F_CPU 16000000UL //clock speed
#define FOSC 16000000UL // system clock frequency
#define BAUD 9600 // baud rate
#define MYUBRR (FOSC/16/BAUD-1) //Define the UBBR value using the equation in datasheet p.173 Table 20-1

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>

#include <stdbool.h> //for boolean
#include <string.h> //for strcpy function

#define SLAVE_ADDRESS 0b1010111 // 87 as decimal. This address must be the same as master address.

static void USART_init(uint16_t ubrr)
{
    /* Set baud rate in the USART Baud Rate Registers (UBRR) */
    UBRR0H = (unsigned char) (ubrr >> 8); //see example datasheet p. 176
    UBRR0L = (unsigned char) ubrr;
    
    /* Enable receiver and transmitter on RX0 and TX0 */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0); // see example datasheet p. 176
    
    /* Set frame format: 8 bit data, 2 stop bit */
    UCSR0C |= (1 << USBS0) | (3 << UCSZ00); //see example datasheet p. 176
    
}

static void USART_Transmit(unsigned char data, FILE *stream)
{
    /* Wait until the transmit buffer is empty*/
    while(!(UCSR0A & (1 << UDRE0))) //see example datasheet p.177
    {
        ;
    }
    
    /* Puts the data into a buffer, then sends/transmits the data */
    UDR0 = data;
}

static char USART_Receive(FILE *stream)
{
    /* Wait until the transmit buffer is empty*/
    while(!(UCSR0A & (1 << RXC0)))  //see example datasheet p. 180
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
    
    // redirect the stdin and stdout to UART functions, read  https://appelsiini.net/2011/simple-usart-with-avr-libc/
    stdout = &uart_output;
    stdin = &uart_input;
    
    TWCR |= (1 << TWEA) | (1 << TWEN);
    TWCR &= ~(1 << TWSTA) & ~(1 << TWSTO);
    
    TWAR = 0b10101110; // 7-bit slave address and 1 write bit (LSB)
    
    char twi_receive_data[20]; // Use TWI instead of SPI
    char test_char_array[16]; // 16-bit array, assumes that the int given is 16-bits
    uint8_t twi_index = 0;
    uint8_t twi_status = 0;
    
    while (1) 
    {
        while(!(TWCR & (1 << TWINT)))
        {;}
            
        twi_status = (TWSR & 0xF8);
        
        TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
        
        while(!(TWCR & (1 << TWINT)))
        {;}
        
        twi_status = (TWSR & 0xF8);
        
        if((twi_status == 0x80) || (twi_status == 0x90))
        {
            twi_receive_data[twi_index] = TWDR;
            twi_index++;
        } else if((twi_status == 0x88) || (twi_status == 0x98))
        {
            twi_receive_data[twi_index] = TWDR;
            twi_index++;
        } else if(twi_status == 0xA0)
        {
            TWCR |= (1 << TWINT);
        }
        
        if (20 <= twi_index)
        {
            printf(twi_receive_data);
            twi_index = 0;
        }
        
    }
}

