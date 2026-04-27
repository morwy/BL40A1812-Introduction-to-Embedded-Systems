/*
 * Mega_UART.c
 * Source : https://appelsiini.net/2011/simple-usart-with-avr-libc/
 *          https://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
 *          https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf 
 */ 

#include "mcu.h"
#include <stdint.h>
#include "avr_gpio.h"
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "uart.h"
#include "pin_config.h"
#include "keypad.h"
#include "lcd.h"
#include "i2c_protocol.h"

#define DOOR_OPENING_DURATION_MS (3000)
#define DOOR_CLOSING_DURATION_MS (2000)
#define DOOR_OPEN_DURATION_MS (1000)
#define FLOOR_MOVING_SPEED_MS (3000)
#define FAULT_BLINK_PERIOD_MS (200)
#define FAULT_BLINK_DURATION_MS (3000)

#define MAX_TEXT_LENGTH (LCD_LINES * LCD_DISP_LENGTH)

#define SLAVE_ADDRESS UNO_I2C_ADDRESS

typedef enum
{
	IDLE = 0,
	GOING_UP = 1,
	GOING_DOWN = 2,
	DOOR_OPENING = 3,
	DOOR_CLOSING = 4,
	FAULT = 5,
	OBSTACLE_DETECTION = 6
} state_t;

uint8_t floor_choice_index = 0; // for testing
int floors_list[] = {5, 3, 3, 1}; // for testing

static uint16_t door_open_elapsed_ms = 0;
static uint8_t obstacle_status = STATUS_CLEAR;

// Helper function for error handling: if return code is non-zero, print error message and halt the system.
static void handle_error(uint8_t return_code)
{
	// Non-zero return code indicates critical fault
	if (return_code != 0)
	{
		printf("%s(): Critical error occurred, return code: %d\r\n", __FUNCTION__, return_code);
		while (1)
		{
		}
	}
}

static void twi_master_init(void)
{
	// Fixed bit rate and prescaler=1
	TWBR = 72;
	TWSR = 0x00;
	TWCR |= (1 << TWEN);
}

static void twi_master_write_to_slave(uint8_t command)
{
	// START -> SLA+W -> COMMAND -> STOP
	uint8_t twi_status = 0;
	char test_char_array[16]; // 16-bit array, assumes that the int given is 16-bits

	// 1) START
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" ");

	// 2) SLA+W
	TWDR = ((SLAVE_ADDRESS << 1) | 0); // address + 0
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" ");

	// 3) COMMAND
	TWDR = command;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" ");

	// 4) STOP
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	printf("\n");
	return;
}

static uint8_t twi_master_read_from_slave(void)
{
	// START -> SLA+R -> read 1 byte (NACK) -> STOP
	uint8_t twi_status = 0;
	uint8_t data;
	char test_char_array[16]; // 16-bit array, assumes that the int given is 16-bits

	// 1) START
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" ");

	// 2) SLA+R
	TWDR = ((SLAVE_ADDRESS << 1) | 1); // address + 1
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" ");

	// 3) Read one byte
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" ");

	data = TWDR;

	// 4) STOP
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	printf("\n");

	return data;
}

static int8_t floor_choice(void) //testing purposes before implementing the keypad
{
	if (floor_choice_index > 3) {
		return 0;
	}
	int floor = floors_list[floor_choice_index];
	floor_choice_index++;
	return floor;
}

state_t idle_state_transition_check(const int8_t requested_floor, const int8_t current_floor)
{
	// Keep state when no buttons are pressed
	if (requested_floor < MIN_FLOOR)
	{
		return IDLE;
	}

	// If invalid input -> go to FAULT;
	if ((requested_floor > MAX_FLOOR) || (requested_floor == current_floor))
	{
		return FAULT;
	}

	// If valid movement request -> go to DOOR_CLOSING;
	if (requested_floor != current_floor)
	{
		return DOOR_CLOSING;
	}

	return IDLE;
}

static void on_enter(state_t new_state, int8_t *requested_floor, int8_t *current_floor)
{
	printf("on_enter new_state: %d\r\n", new_state);
	switch (new_state)
	{
	case IDLE:
	{
        // Display "Choose the floor" on LCD screen:
		// code here
		break;
	}
	case GOING_UP:
	{
		set_gpio(&movement_led); // turn movement LED ON
        // Display "Current floor: XX" on LCD screen:
        // code here
		break;
	}
	case GOING_DOWN:
	{
		set_gpio(&movement_led); // turn movement LED ON
		// Display "Current floor: XX" on LCD screen:
		// code here
		break;
	}
	case DOOR_OPENING:
	{
		set_gpio(&doors_led);
		// Door LED is on for 3 seconds, but we time-slice in on_loop
		// so we can poll the UNO obstacle status 10 times
		door_open_elapsed_ms = 0;
        // Display "Door open" on LCD screen:
		// code here
		break;
	}
	case DOOR_CLOSING:
	{
        set_gpio(&doors_led);
        // Display "Door closing" on LCD screen:
		// code here
		_delay_ms(DOOR_CLOSING_DURATION_MS); // door led is one for 2 seconds
		break;
	}
	case FAULT:
	{
		// FAULT RECOVERY
		if (*current_floor < MIN_FLOOR)
		{
			*current_floor = MIN_FLOOR;
		}

		if (*current_floor > MAX_FLOOR)
		{
			*current_floor = MAX_FLOOR;
		}

		// reset requested floor
		*requested_floor = FLOOR_NOT_SELECTED;
		
		// Display "Same floor" on LCD screen to indicate fault:
		// code here
		break;
	}
    case OBSTACLE_DETECTION:
	{
		// Obstacle detected: obstacle led blinks 3 times, LCD displays "Obstacle detected", buzzer plays melody with 5 notes, stops until any button on the keypad is pressed
        // code here
		// - TODO (LCD): display "Obstacle detected"
		
		// Tell UNO to start blinking + buzzer/melody
		printf("Sending start command to UNO\r\n");
		twi_master_write_to_slave(CMD_OBSTACLE_ON);
		printf("Start command sent to UNO\r\n");

		// Wait for any keypad press to stop
		/* while (1)
		{
			uint8_t key = KEYPAD_GetKey();
			if (key != 0)
			{
				break;
			}
		} */

		_delay_ms(3000); // for testing, wait for 3 seconds

        break;
	}
	}
}

static void on_loop(state_t current_state, int8_t *requested_floor, int8_t *current_floor)
{
	printf("on_loop current_state: %d\r\n", current_state);
	switch (current_state)
	{
	case IDLE:
	{
		_delay_ms(10);
		*requested_floor = floor_choice();
		break;
	}
	case GOING_UP:
	{
		_delay_ms(FLOOR_MOVING_SPEED_MS);
		(*current_floor)++;
		// Display new "Current floor: XX" on LCD screen:
		// code here
		break;
	}
	case GOING_DOWN:
	{
		(*current_floor)--;
		// Display new "Current floor: XX" on LCD screen:
		// code here
		_delay_ms(FLOOR_MOVING_SPEED_MS);
		break;
	}
	case DOOR_OPENING:
	{
		// Poll UNO obstacle status during the door-open period.
		printf("Polling UNO obstacle status\r\n");
		obstacle_status = twi_master_read_from_slave();
		printf("UNO obstacle status: %d\r\n", obstacle_status);

		// Time slice: 300ms tick
		_delay_ms(300);
		door_open_elapsed_ms += 300;


		// Testing, set obstacle flag to 1
		if (door_open_elapsed_ms >= 900)
		{
			obstacle_status = STATUS_OBSTACLE;
		}
		
		break;
	}
	}
}

static void on_exit(state_t old_state, int8_t *requested_floor, int8_t *current_floor)
{
	printf("on_exit old_state: %d\r\n", old_state);
	switch (old_state)
	{
	case GOING_UP:
	case GOING_DOWN:
	{
		clear_gpio(&movement_led);
		break;
	}
	case DOOR_CLOSING:
	{
		clear_gpio(&doors_led);
		break;
	}
	case DOOR_OPENING:
    {
		clear_gpio(&doors_led);
		break;
	}
	case OBSTACLE_DETECTION:
	{
		// Tell UNO to stop the buzzer/melody when leaving obstacle detection
		printf("Sending stop command to UNO\r\n");
		twi_master_write_to_slave(CMD_OBSTACLE_OFF);
		printf("Stop command sent to UNO\r\n");

		// Reset obstacle status
		obstacle_status = STATUS_CLEAR;

		break;
	}
    }
}

int main(void)
{
    uint8_t rc = setup_uart_io();
    handle_error(rc);

	// Configuring GPIO mappings
	init_avr_gpio_pins();

	// Initializing movement and door LEDs.
	clear_gpio(&movement_led);
	set_as_output(&movement_led);
	clear_gpio(&doors_led);
	set_as_output(&doors_led);

	// Initialize I2C/TWI master 
	twi_master_init();

    /* elevator variables, elevator has 5 floors */
	volatile state_t elevator_state = IDLE;
	int8_t requested_floor = FLOOR_NOT_SELECTED;
	int8_t current_floor = 1;

	while (1)
	{
		state_t next_state = FAULT;
		/* State machine - switch case */
		switch (elevator_state)
		{
		case IDLE:
		{
			next_state = idle_state_transition_check(requested_floor, current_floor);
			break;
		}
		case GOING_UP:
		case GOING_DOWN:
		{
			next_state = elevator_state;
			if (requested_floor == current_floor)
			{
				// FLOOR REACHED
				next_state = DOOR_OPENING;
			}
			break;
		}
		case DOOR_OPENING:
		{
			next_state = DOOR_OPENING;

			// If obstacle is detected at any time during this open period, enter OBSTACLE_DETECTION.
			if (obstacle_status == STATUS_OBSTACLE)
			{
				next_state = OBSTACLE_DETECTION;
			}
			// Otherwise, after DOOR_OPEN_DURATION_MS, proceed to DOOR_CLOSING.
			else if (door_open_elapsed_ms >= DOOR_OPEN_DURATION_MS)
			{
				next_state = DOOR_CLOSING;
			}
			break;
		}
		case DOOR_CLOSING:
		{
			if (requested_floor < current_floor)
			{
				next_state = GOING_DOWN;
			}
			else if (requested_floor > current_floor)
			{
				next_state = GOING_UP;
			}
			else
			{
                requested_floor = FLOOR_NOT_SELECTED; //reset requested floor before going IDLE
				next_state = IDLE;
			}
			break;
		}
		case FAULT:
		{
			// WRONG FLOOR INPUT. RECOVERY ON ENTER, THEN PROCEED TO IDLE
			next_state = IDLE;
			break;
		}
        case OBSTACLE_DETECTION:
		{
			// Obstacle detected: obstacle led blinks 3 times, LCD displays "Obstacle detected", buzzer plays melody with 5 notes, stops until any button on the keypad is pressed
            
			next_state = DOOR_CLOSING;

            break;
		}
		}
		printf("elevator_state: %d "
			   "next_state: %d "
			   "requested_floor: %d, "
			   "current_floor: %d\r\n",
			   elevator_state,
			   next_state,
			   requested_floor,
			   current_floor);
		if (elevator_state == next_state)
		{
			on_loop(elevator_state, &requested_floor, &current_floor);
		}
		else
		{
			on_exit(elevator_state, &requested_floor, &current_floor);
			elevator_state = next_state;
			on_enter(elevator_state, &requested_floor, &current_floor);
		}
	}
 
    return EXIT_SUCCESS;
}
