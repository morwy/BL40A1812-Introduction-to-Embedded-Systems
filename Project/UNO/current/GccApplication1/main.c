#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stdbool.h>

#include "pin_config.h"
#include "i2c_protocol.h"
#include "buzzer.h"
#include "hcsr04.h"

// --- GLOBAL VARIABLES ---
volatile bool play_melody = false;
volatile uint8_t obstacle_status = STATUS_CLEAR;

// --- I2C (TWI) INTERRUPT SERVICE ROUTINE ---
ISR(TWI_vect) {
	switch (TW_STATUS) {
		
		// Master gives us a sock
		case TW_SR_DATA_ACK:
		{
			uint8_t command = TWDR; 
			
			if (command == CMD_OBSTACLE_ON) {
				OBSTACLE_PORT |= (1 << OBSTACLE_PIN);
				//buzzer_start_melody();
			}
			else if (command == CMD_OBSTACLE_OFF) {
				OBSTACLE_PORT &= ~(1 << OBSTACLE_PIN);
				//buzzer_stop_melody();
			}
			else if (command == CMD_BUZZER_START) {
				buzzer_start_melody(); 			
			}
			else if (command == CMD_BUZZER_STOP) {
				buzzer_stop_melody();  			
			}
			break;
		}

		// We give master a sock
		case TW_ST_SLA_ACK:   
		case TW_ST_DATA_ACK:  
		{
			TWDR = obstacle_status; 
			break;
		}
		
		default:
		break;
	}
	
	// Clear the interrupt flag and enable Acknowledgement to ready the bus for the next action
	TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWEA);
}

// --- HARDWARE INITIALIZATION ---
void init_hardware(void) {
	OBSTACLE_DDR |= (1 << OBSTACLE_PIN);
	OBSTACLE_PORT &= ~(1 << OBSTACLE_PIN);

	buzzer_init();
	hcsr04_init();

	TWAR = (UNO_I2C_ADDRESS << 1);
	
	// TWCR = control switches
	// TWEN = Two-Wire ENable (Turns on I2C)
	// TWIE = Two-Wire Interrupt Enable (allows I2c to trigger ISR(TwI_vect))
	// TWEA = Two-Wire Enable Acknowledge (UNO sends ACK back automatically)
	// TWINT = Two-Wire INTerrupt (readies the hardware for the first message)
	TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 << TWINT);
	
	//Set Enable Interrupts
	sei();
}

int main(void) {
	init_hardware();

	// Testing
	OBSTACLE_PORT ^= (1 << OBSTACLE_PIN);
	_delay_ms(1000);
	OBSTACLE_PORT ^= (1 << OBSTACLE_PIN);

	while (1) {
		
		//hcsr04_update();
		
		//buzzer_update();
	}
	
	return 0;
}