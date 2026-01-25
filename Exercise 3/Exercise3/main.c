#include "mcu.h"
#include <stdint.h>
#include "avr_gpio.h"
#include <util/delay.h>

#include "uart.h"
#include "pin_config.h"

#define DOOR_CLOSE_OPEN_DURATION_MS (3000)
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
} state_t;

static int8_t floor_button_choice(void)
{
	for (int8_t i = MIN_FLOOR; i <= MAX_FLOOR; i++)
	{
		if (!read_gpio(&floor_buttons[i - MIN_FLOOR]))
		{
			return i;
		}
	}
	return MIN_FLOOR - 1;
}

static void floor_led_on(const int8_t current_floor)
{
	...;
	set_gpio(&floor_leds[current_floor - MIN_FLOOR]);
}

static void floor_led_off(const int8_t current_floor)
{
	...;
}

state_t idle_state_transition_check(const int8_t requested_floor, const int8_t current_floor)
{
	// Keep state when no buttons are pressed
	if (requested_floor == (MIN_FLOOR - 1))
	{
		return IDLE;
	}
	
	// If invalid input -> go to FAULT;
	if ((requested_floor > MAX_FLOOR) || (requested_floor < MIN_FLOOR))
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
		break;
	case GOINGUP:
		set_gpio(&movement_led); // turn movement LED ON
		break;
	case GOINGDOWN:
		set_gpio(&movement_led); // turn movement LED ON
		break;
	case DOOR_OPENING:
		set_gpio(&doors_led);
		_delay_ms(DOOR_CLOSE_OPEN_DURATION_MS);
		break;
	case DOOR_CLOSING:
		break;
	case FAULT:
		// FAULT RECOVERY
		if (*current_floor < MIN_FLOOR)
			*current_floor = MIN_FLOOR;
		if (*current_floor > MAX_FLOOR)
			*current_floor = MAX_FLOOR;
		*requested_floor = *current_floor;
		// Blink movement LED to indicate fault.
		for (int8_t blink_amount = 0; blink_amount < (FAULT_BLINK_DURATION_MS/FAULT_BLINK
		{
			set_gpio(&movement_led);
			_delay_ms(FAULT_BLINK_PERIOD_MS / 2.);
			clear_gpio(&movement_led);
			_delay_ms(FAULT_BLINK_PERIOD_MS / 2.);
		}
		break;
	}
}

static void on_loop(state_t current_state, int8_t *requested_floor, int8_t *current_floor)
{
	printf("on_loop current_state: %d\r\n", current_state);
	...;
}

static void on_exit(state_t old_state, int8_t *requested_floor, int8_t *current_floor)
{
	...;
}

/// There is not much we can do for now. This function will be improved in future.
static void handle_error(uint8_t return_code)
{
	// Non-zero return code indicates critical fault
	if (return_code)
	{
		while (1)
			;
	}
}

int main(void)
{
	// Initialize serial port for standard library
	uint8_t rc = setup_uart_io();
	handle_error(rc);
	printf("Configuring GPIO mapping.\r\n");
	init_avr_gpio_pins();
	printf("Configuring IO.\r\n");
	for (int8_t i = MIN_FLOOR; i <= MAX_FLOOR; i++)
	{
		// Default off
		clear_gpio(&floor_leds[i - MIN_FLOOR]);
		set_as_output(&floor_leds[i - MIN_FLOOR]);
		set_as_input_pull_up(&floor_buttons[i - MIN_FLOOR]);
	}
	
	clear_gpio(&movement_led);
	set_as_output(&movement_led);
	set_gpio(&doors_led);
	set_as_output(&doors_led);
	printf("Configuring IO done.\r\n");
	
	/* elevator variables, elevator has 5 floors */
	volatile state_t elevator_state = IDLE;
	int8_t requested_floor = 1;
	int8_t current_floor = 1;
	set_gpio(&doors_led);
	floor_led_on(current_floor);
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
			if (requested_floor == current_floor)
			{
				// FLOOR REACHED
				next_state = DOOR_OPENING;
			}
			break;
		case DOOR_OPENING:
			next_state = IDLE;
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
				next_state = IDLE;
			}
			break;
		case FAULT:
			// WRONG FLOOR INPUT. RECOVERY ON ENTER, THEN PROCEED TO IDLE
			next_state = IDLE;
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
}