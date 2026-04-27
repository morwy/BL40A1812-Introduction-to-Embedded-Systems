#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/// GPIO pin mappings from MCU ports to Arduino UNO digital IO pins

#define IO_3_PORT (PORTE)
#define IO_3_DIRECTION (DDRE)
#define IO_3_INPUT (PINE)
#define IO_3_PIN (PE5)

#define IO_5_PORT (PORTE)
#define IO_5_DIRECTION (DDRE)
#define IO_5_INPUT (PINE)
#define IO_5_PIN (PE3)

#define IO_6_PORT (PORTH)
#define IO_6_DIRECTION (DDRH)
#define IO_6_INPUT (PINH)
#define IO_6_PIN (PH3)

#define IO_7_PORT (PORTH)
#define IO_7_DIRECTION (DDRH)
#define IO_7_INPUT (PINH)
#define IO_7_PIN (PH4)

#define IO_8_PORT (PORTH)
#define IO_8_DIRECTION (DDRH)
#define IO_8_INPUT (PINH)
#define IO_8_PIN (PH5)

#define IO_9_PORT (PORTH)
#define IO_9_DIRECTION (DDRH)
#define IO_9_INPUT (PINH)
#define IO_9_PIN (PH6)

#define IO_10_PORT (PORTB)
#define IO_10_DIRECTION (DDRB)
#define IO_10_INPUT (PINB)
#define IO_10_PIN (PB4)

#define IO_11_PORT (PORTB)
#define IO_11_DIRECTION (DDRB)
#define IO_11_INPUT (PINB)
#define IO_11_PIN (PB5)

#define IO_11_PORT (PORTB)
#define IO_11_DIRECTION (DDRB)
#define IO_11_INPUT (PINB)
#define IO_11_PIN (PB5)

#define IO_22_PORT (PORTA)
#define IO_22_DIRECTION (DDRA)
#define IO_22_INPUT (PINA)
#define IO_22_PIN (PA0)

#define IO_23_PORT (PORTA)
#define IO_23_DIRECTION (DDRA)
#define IO_23_INPUT (PINA)
#define IO_23_PIN (PA1)

#define IO_24_PORT (PORTA)
#define IO_24_DIRECTION (DDRA)
#define IO_24_INPUT (PINA)
#define IO_24_PIN (PA2)

#define IO_25_PORT (PORTA)
#define IO_25_DIRECTION (DDRA)
#define IO_25_INPUT (PINA)
#define IO_25_PIN (PA3)

#define IO_26_PORT (PORTA)
#define IO_26_DIRECTION (DDRA)
#define IO_26_INPUT (PINA)
#define IO_26_PIN (PA4)

// MOVMENT & DOORS LEDS MAPPINGS:

#define IO_31_PORT (PORTC)
#define IO_31_DIRECTION (DDRC)
#define IO_31_INPUT (PINC)
#define IO_31_PIN (PC6)

#define IO_32_PORT (PORTC)
#define IO_32_DIRECTION (DDRC)
#define IO_32_INPUT (PINC)
#define IO_32_PIN (PC5)

//////////////////////////////////


#define IO_33_PORT (PORTC)
#define IO_33_DIRECTION (DDRC)
#define IO_33_INPUT (PINC)
#define IO_33_PIN (PC4)

#define IO_34_PORT (PORTC)
#define IO_34_DIRECTION (DDRC)
#define IO_34_INPUT (PINC)
#define IO_34_PIN (PC3)

#define IO_35_PORT (PORTC)
#define IO_35_DIRECTION (DDRC)
#define IO_35_INPUT (PINC)
#define IO_35_PIN (PC2)

#define IO_36_PORT (PORTC)
#define IO_36_DIRECTION (DDRC)
#define IO_36_INPUT (PINC)
#define IO_36_PIN (PC1)

#define IO_37_PORT (PORTC)
#define IO_37_DIRECTION (DDRC)
#define IO_37_INPUT (PINC)
#define IO_37_PIN (PC0)
#endif