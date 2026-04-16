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

#include "uart.h"
#include "pin_config.h"

#define DOOR_OPEN_DURATION_MS (3000)
#define DOOR_CLOSE_DURATION_MS (2000)
#define FLOOR_MOVING_SPEED_MS (3000)
#define FAULT_BLINK_PERIOD_MS (200)
#define FAULT_BLINK_DURATION_MS (3000)


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

uint8_t floor_choice_index = 0;
int floors_list[] = {5, 3, 3, 1};

/// There is not much we can do for now. This function will be improved in future.
static void handle_error(uint8_t return_code)
{
	// Non-zero return code indicates critical fault
	if (return_code)
	{
	    while(1);
	}
}

static int8_t floor_choice(void) //THIS IS FOR SIMULATING THE FLOOR CHOICE. IN REALITY THE CHOICE COMES FROM THE KEYPAD INPUT, WHICH IS NOT IMPLEMENTED YET.
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
		_delay_ms(DOOR_OPEN_DURATION_MS); // door led is one for 3 seconds
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
		clear_gpio(&doors_led);
		break;
	case DOOR_OPENING:
        clear_gpio(&doors_led);
        break;
    }
}

int main(void)
{
    uint8_t rc = setup_uart_io();
    handle_error(rc);

	init_avr_gpio_pins();

	//testing led pin mappings
	set_gpio(&doors_led);
	_delay_ms(2000);
	clear_gpio(&doors_led);
	set_gpio(&movement_led);
	_delay_ms(2000);
	clear_gpio(&movement_led);

    //char username[32] = {0};

    // print out using the UART. This can be accessed via terminals such as PuTTY.
    /*
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
	*/

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
			next_state = DOOR_CLOSING;
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
            //

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
