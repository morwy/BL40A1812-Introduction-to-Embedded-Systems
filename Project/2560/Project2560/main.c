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

#define DOOR_OPEN_DURATION_MS (3000)
#define DOOR_CLOSE_DURATION_MS (2000)
#define FLOOR_MOVING_SPEED_MS (3000)
#define FAULT_BLINK_PERIOD_MS (200)
#define FAULT_BLINK_DURATION_MS (3000)

#define MAX_TEXT_LENGTH (LCD_LINES * LCD_DISP_LENGTH)

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

// Keypad input variables
static uint8_t input_digits[2];
static uint8_t input_index = 0;

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

// Helper function for displaying text on LCD screen, with some error handling.
static void lcd_show_text(const char *text)
{
	if (text == NULL)
	{
		printf("%s(): provided text is NULL!\r\n", __FUNCTION__);
		return;
	}

	if (strlen(text) > MAX_TEXT_LENGTH)
	{
		printf("%s(): provided text is longer than %d characters, it will be truncated.\r\n", __FUNCTION__, MAX_TEXT_LENGTH);
	}

	lcd_clrscr();
	lcd_puts(text);

	printf("%s(): displayed text \"%s\"\r\n", __FUNCTION__, text);
}

static int8_t floor_choice(void)
{
	const uint8_t key = KEYPAD_GetKey();
	printf("%s(): \"%c\" key was entered\r\n", __FUNCTION__, key);

	if (key >= '0' && key <= '9')
	{
		input_digits[input_index] = key - '0';
		input_index++;

		char buf[MAX_TEXT_LENGTH];

		if (input_index == 1)
		{
			sprintf(buf, "Select floor:   \r\n%d", input_digits[0]);
			lcd_show_text(buf);
		}
		else if (input_index == 2)
		{
			int floor = input_digits[0] * 10 + input_digits[1];
			input_index = 0; // reset for next input

			sprintf(buf, "Select floor:   \r\n%d%d", input_digits[0], input_digits[1]);
			lcd_show_text(buf);

			if (floor >= MIN_FLOOR && floor <= MAX_FLOOR)
			{
				return floor;
			}
			else
			{
				// invalid, reset
				return FLOOR_NOT_SELECTED;
			}
		}
	}
	else if (key == '#')
	{
		if (input_index == 1)
		{
			int floor = input_digits[0];
			input_index = 0;
			return floor;
		}
		else if (input_index == 2)
		{
			int floor = input_digits[0] * 10 + input_digits[1];
			input_index = 0;
			return floor;
		}
		// if no digits, ignore
	}

	return FLOOR_NOT_SELECTED; // not ready or invalid
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
	printf("%s(): new_state: %d\r\n", __FUNCTION__, new_state);

	switch (new_state)
	{
	case IDLE:
	{
		lcd_show_text("Select floor:   ");
		break;
	}
	case GOING_UP:
	{
		set_gpio(&movement_led); // turn movement LED ON

		char buf[MAX_TEXT_LENGTH];
		sprintf(buf, "Current floor:  \r\n%d", *current_floor);
		lcd_show_text(buf);

		break;
	}
	case GOING_DOWN:
	{
		set_gpio(&movement_led); // turn movement LED ON

		char buf[MAX_TEXT_LENGTH];
		sprintf(buf, "Current floor:  \r\n%d", *current_floor);
		lcd_show_text(buf);

		break;
	}
	case DOOR_OPENING:
	{
		set_gpio(&doors_led);
		_delay_ms(DOOR_OPEN_DURATION_MS); // door led is one for 3 seconds
		lcd_show_text("Door open");
		break;
	}
	case DOOR_CLOSING:
	{
		set_gpio(&doors_led);
		lcd_show_text("Door closing");
		_delay_ms(DOOR_CLOSE_DURATION_MS); // door led is one for 2 seconds
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

		lcd_show_text("Same floor");
		break;
	}
	case OBSTACLE_DETECTION:
	{
		// Obstacle detected: obstacle led blinks 3 times, LCD displays "Obstacle detected", buzzer plays melody with 5 notes, stops until any button on the keypad is pressed
		lcd_show_text("Obstacle detected");

		// TODO: Blink obstacle LED 3 times
		// TODO: Play buzzer melody (5 notes)

		// Wait for any keypad press to stop
		while (1)
		{
			uint8_t key = KEYPAD_GetKey();
			if (key != 0)
			{
				break;
			}
		}
		break;
	}
	}
}

static void on_loop(state_t current_state, int8_t *requested_floor, int8_t *current_floor)
{
	printf("%s(): current_state: %d\r\n", __FUNCTION__, current_state);

	switch (current_state)
	{
	case IDLE:
	{
		_delay_ms(10);

		int8_t floor = floor_choice();
		if (floor >= 0)
		{
			*requested_floor = floor;
		}
		break;
	}
	case GOING_UP:
	{
		_delay_ms(FLOOR_MOVING_SPEED_MS);
		(*current_floor)++;

		char buf[MAX_TEXT_LENGTH];
		sprintf(buf, "Current floor:  \r\n%d", *current_floor);
		lcd_show_text(buf);
		break;
	}
	case GOING_DOWN:
	{
		(*current_floor)--;

		char buf[MAX_TEXT_LENGTH];
		sprintf(buf, "Current floor:  \r\n%d", *current_floor);
		lcd_show_text(buf);
		_delay_ms(FLOOR_MOVING_SPEED_MS);
		break;
	}
	}
}

static void on_exit(state_t old_state, int8_t *requested_floor, int8_t *current_floor)
{
	printf("%s(): old_state: %d\r\n", __FUNCTION__, old_state);

	switch (old_state)
	{
	case GOING_UP:
	case GOING_DOWN:
	{
		clear_gpio(&movement_led);
		break;
	}
	case DOOR_CLOSING:
	case DOOR_OPENING:
	{
		clear_gpio(&doors_led);
		break;
	}
	}
}

int main(void)
{
	// Initialize UART communication and handle errors
	uint8_t rc = setup_uart_io();
	handle_error(rc);

	// Configuring GPIO mappings
	init_avr_gpio_pins();

	// Initialize keypad and LCD
	KEYPAD_Init();
	lcd_init(LCD_DISP_ON);

	// Initializing movement and door LEDs.
	clear_gpio(&movement_led);
	set_as_output(&movement_led);
	clear_gpio(&doors_led);
	set_as_output(&doors_led);

	// Elevator part
	volatile state_t elevator_state = IDLE;
	int8_t requested_floor = FLOOR_NOT_SELECTED;
	int8_t current_floor = 1;

	// State machine - switch case
	on_enter(elevator_state, &requested_floor, &current_floor);

	while (1)
	{
		state_t next_state = FAULT;

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
			next_state = DOOR_CLOSING;
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
				// reset requested floor before going IDLE
				requested_floor = FLOOR_NOT_SELECTED;
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
