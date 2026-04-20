/*
 * Mega_UART.c
 * Source : https://appelsiini.net/2011/simple-usart-with-avr-libc/
 *          https://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
 *          https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf 
 */ 

#include "mcu.h"
#include <avr/io.h>
#include <stdint.h>
#include "avr_gpio.h"
#include <util/delay.h>
#include <stdio.h>

#include "uart.h"
#include "pin_config.h"

#define DOOR_OPEN_DURATION_MS (3000)
#define DOOR_CLOSE_DURATION_MS (2000)
#define FLOOR_MOVING_SPEED_MS (3000)
#define FAULT_BLINK_PERIOD_MS (200)
#define FAULT_BLINK_DURATION_MS (3000)

// Must match UNO slave address.
#define SLAVE_ADDRESS_7BIT (0x57)

// Simple 1-byte command protocol to UNO
#define UNO_CMD_OBSTACLE_START (0x01) // start blinking + buzzer/melody
#define UNO_CMD_OBSTACLE_STOP  (0x02) // stop buzzer/melody 

static void twi_master_init(void)
{
	// Fixed bit rate and prescaler=1
	TWBR = 0x03;
	TWSR = 0x00;
	TWCR |= (1 << TWEN);
}

static uint8_t twi_master_write_to_slave(uint8_t slave_address_7bit, uint8_t data_byte)
{
	// START -> SLA+W -> DATA -> STOP
	uint8_t twi_status = 0;
	//char test_char_array[16]; // 16-bit array, assumes that the int given is 16-bits

	// 1) START
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	/* itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" "); */

	// 2) SLA+W
	TWDR = ((slave_address_7bit << 1) | 0);
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	/* itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" "); */

	// 3) DATA
	TWDR = data_byte;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	/* itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" "); */

	// 4) STOP
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	/* printf("\n"); */
	return 0;
}

static uint8_t twi_master_read_from_slave(uint8_t slave_address_7bit, uint8_t *out_byte)
{
	// START -> SLA+R -> read 1 byte (NACK) -> STOP
	uint8_t twi_status = 0;
	//char test_char_array[16]; // 16-bit array, assumes that the int given is 16-bits

	// 1) START
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	/* itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" "); */

	// 2) SLA+R
	TWDR = ((slave_address_7bit << 1) | 1);
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	/* itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" "); */

	// 3) Read one byte
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
	{;}
	twi_status = (TWSR & 0xF8);
	/* itoa(twi_status, test_char_array, 16);
	printf(test_char_array);
	printf(" "); */

	*out_byte = TWDR;

	// 4) STOP
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	/* printf("\n"); */

	return 0;
}

static uint8_t is_keypad_any_pressed(void)
{
	// TODO (keypad): return 1 when any keypad button is pressed.
	// This is used to exit OBSTACLE_DETECTION.
	return 0;
}


typedef enum
{
	IDLE = 0,
	GOINGUP = 1,
	GOINGDOWN = 2,
	DOOR_OPENING = 3,
	DOOR_CLOSING = 4,
	FAULT = 5,
    OBSTACLE_DETECTION = 6
} state_t;

uint8_t floor_choice_index = 0; // for testing
int floors_list[] = {5, 3, 3, 1}; // for testing

static uint16_t door_open_elapsed_ms = 0;
static uint8_t obstacle_flag = 0;

/// There is not much we can do for now. This function will be improved in future.
static void handle_error(uint8_t return_code)
{
	// Non-zero return code indicates critical fault
	if (return_code)
	{
	    while(1);
	}
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
	if (requested_floor == (MIN_FLOOR - 1))
	{
		return IDLE;
	}

	// If invalid input -> go to FAULT;
	if ((requested_floor > MAX_FLOOR) || (requested_floor < MIN_FLOOR) || (requested_floor == current_floor))
	{

		return FAULT;
	}

	if (requested_floor != current_floor)
	{
		// If valid movement request -> go to DOOR_CLOSING;
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
        // Display "Choose the floor" on LCD screen:
		// code here
		break;
	case GOINGUP:
		set_gpio(&movement_led); // turn movement LED ON
        // Display "Current floor: XX" on LCD screen:
        // code here
		break;
	case GOINGDOWN:
		set_gpio(&movement_led); // turn movement LED ON
		// Display "Current floor: XX" on LCD screen:
		// code here
		break;
	case DOOR_OPENING:
		set_gpio(&doors_led);
		// Door LED is on for 3 seconds, but we time-slice in on_loop
		// so we can poll the UNO obstacle flag 10 times during the open period.
		door_open_elapsed_ms = 0;
        // Display "Door open" on LCD screen:
		// code here
		break;
	case DOOR_CLOSING:
        set_gpio(&doors_led);
        // Display "Door closing" on LCD screen:
		// code here
		_delay_ms(DOOR_CLOSE_DURATION_MS); // door led is one for 2 seconds
		break;
	case FAULT:
		// FAULT RECOVERY
		if (*current_floor < MIN_FLOOR)
			*current_floor = MIN_FLOOR;
		if (*current_floor > MAX_FLOOR)
			*current_floor = MAX_FLOOR;
		*requested_floor = 0; // reset requested floor
		
		// Display "Same floor" on LCD screen to indicate fault:
		// code here
		break;
    case OBSTACLE_DETECTION:
		// Obstacle detected: obstacle led blinks 3 times, LCD displays "Obstacle detected", buzzer plays melody with 5 notes, stops until any button on the keypad is pressed
        // code here
		// - TODO (LCD): display "Obstacle detected"
		
		// Tell UNO to start blinking + buzzer/melody
		printf("Sending start command to UNO\r\n");
		(void)twi_master_write_to_slave(SLAVE_ADDRESS_7BIT, UNO_CMD_OBSTACLE_START);
		printf("Start command sent to UNO\r\n");
        break;
	}
}

static void on_loop(state_t current_state, int8_t *requested_floor, int8_t *current_floor)
{
	printf("on_loop current_state: %d\r\n", current_state);
	switch (current_state)
	{
	case IDLE:
		_delay_ms(10);
		*requested_floor = floor_choice();
		break;
	case GOINGUP:
		_delay_ms(FLOOR_MOVING_SPEED_MS);
		(*current_floor)++;
		// Display new "Current floor: XX" on LCD screen:
		// code here
		break;
	case GOINGDOWN:
		(*current_floor)--;
		// Display new "Current floor: XX" on LCD screen:
		// code here
		_delay_ms(FLOOR_MOVING_SPEED_MS);
		break;
	case DOOR_OPENING:
	{
		// Poll UNO obstacle flag during the door-open period.
		uint8_t flag = 0;
		printf("Polling UNO obstacle flag\r\n");
		(void)twi_master_read_from_slave(SLAVE_ADDRESS_7BIT, &flag);
		printf("UNO obstacle flag: %d\r\n", flag);
		obstacle_flag = flag;

		// Time slice: 300ms tick
		_delay_ms(300);
		door_open_elapsed_ms += 300;


		// Testing, set obstacle flag to 1
		if (door_open_elapsed_ms >= 900)
		{
			obstacle_flag = 1;
		}
		
		break;
	}
	case OBSTACLE_DETECTION:
		// Wait here until user acknowledges via keypad.
		
		_delay_ms(25);
		break;
	}
}

static void on_exit(state_t old_state, int8_t *requested_floor, int8_t *current_floor)
{
	printf("on_exit old_state: %d\r\n", old_state);
	switch (old_state)
	{
	case GOINGUP:
	case GOINGDOWN:
		clear_gpio(&movement_led);
		break;
	case DOOR_CLOSING:
	case DOOR_OPENING:
        clear_gpio(&doors_led);
        break;
	case OBSTACLE_DETECTION:
		// Tell UNO to stop the buzzer/melody when leaving obstacle detection
		printf("Sending stop command to UNO\r\n");
		(void)twi_master_write_to_slave(SLAVE_ADDRESS_7BIT, UNO_CMD_OBSTACLE_STOP);
		printf("Stop command sent to UNO\r\n");
		break;
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
	int8_t requested_floor = 0;
	int8_t current_floor = 1;

	while (1)
	{
		state_t next_state = FAULT;
		/* State machine - switch case */
		switch (elevator_state)
		{
		case IDLE:
			next_state = idle_state_transition_check(requested_floor, current_floor);
			break;
		case GOINGUP:
		case GOINGDOWN:
			next_state = elevator_state;
			//printf("Current floor: %d\r\n", current_floor);
			if (requested_floor == current_floor)
			{
				// FLOOR REACHED
				next_state = DOOR_OPENING;
				//printf("Floor %d reached\r\n", current_floor);
			}
			break;
		case DOOR_OPENING:
			// Door is open; poll the UNO obstacle flag.
			// If obstacle is detected at any time during this open period, enter OBSTACLE_DETECTION.
			// Otherwise, after DOOR_OPEN_DURATION_MS, proceed to DOOR_CLOSING.
			next_state = DOOR_OPENING;
			if (obstacle_flag)
			{
				next_state = OBSTACLE_DETECTION;
			}
			else if (door_open_elapsed_ms >= DOOR_OPEN_DURATION_MS)
			{
				next_state = DOOR_CLOSING;
			}
			break;
		case DOOR_CLOSING:
			if (requested_floor < current_floor)
			{
				next_state = GOINGDOWN;
			}
			else if (requested_floor > current_floor)
			{
				next_state = GOINGUP;
			}
			else
			{
                requested_floor = 0; //reset requested floor before going IDLE
				next_state = IDLE;
			}
			break;
		case FAULT:
			// WRONG FLOOR INPUT. RECOVERY ON ENTER, THEN PROCEED TO IDLE
			next_state = IDLE;
			break;
        case OBSTACLE_DETECTION:
			// Obstacle detected: obstacle led blinks 3 times, LCD displays "Obstacle detected", buzzer plays melody with 5 notes, stops until any button on the keypad is pressed
            // code here
            // Stay here until any keypad button is pressed, then continue to DOOR_CLOSING.
			next_state = OBSTACLE_DETECTION;

			if (is_keypad_any_pressed())
			{
				next_state = DOOR_CLOSING;
			}

			// Testing, keypad pressed
			next_state = DOOR_CLOSING;

            break;
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
 
    return 0;
}
