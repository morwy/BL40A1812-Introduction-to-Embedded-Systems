/*
 * UNO_EEPROM.c

 */ 

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

    // redirect the stdin and stdout to UART functions,  read  https://appelsiini.net/2011/simple-usart-with-avr-libc/
    stdout = &uart_output;
    stdin = &uart_input;

    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);

    unsigned char spi_send_data[20] = " ...write what to transfer to slave... \n\r"; //to slave
    unsigned char spi_receive_data[20]; //from slave

    while (true) {
        PORTB &= ~(1 << PB0); // SS low, enable the slave device

        for(int8_t spi_data_index = 0; spi_data_index < sizeof(spi_send_data); spi_data_index++)
        //to read data from slave
        {
            SPDR = spi_send_data[spi_data_index]; // Use SPI data register (SPDR) to send byte of data
            /* wait until the transmission is complete */
            while(!(SPSR & (1 << SPIF))) //see example datasheet p.193, and datasheet at p.198.
            {
                ;
            }
            spi_receive_data[spi_data_index] = SPDR; // receive byte from the SPI data register
        }
        PORTB |= (1 << PB0); // SS HIGH to disable the slave device
        printf(spi_receive_data);
    }
    
}