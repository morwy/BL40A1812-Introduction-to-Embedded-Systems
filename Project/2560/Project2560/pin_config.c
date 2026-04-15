#include "pin_config.h"

avr_gpio_t floor_buttons[MAX_FLOOR-MIN_FLOOR+1];
avr_gpio_t floor_leds[MAX_FLOOR-MIN_FLOOR+1];
avr_gpio_t doors_led;
avr_gpio_t movement_led;

void init_avr_gpio_pins(void) {
	floor_buttons[1-MIN_FLOOR].port = &BUTTON_FLOOR_1_PORT;
	floor_buttons[1-MIN_FLOOR].input = &BUTTON_FLOOR_1_INPUT;
	floor_buttons[1-MIN_FLOOR].direction = &BUTTON_FLOOR_1_DIRECTION;
	floor_buttons[1-MIN_FLOOR].pin_offset = BUTTON_FLOOR_1_PIN;
	
	floor_buttons[2-MIN_FLOOR].port = &BUTTON_FLOOR_2_PORT;
	floor_buttons[2-MIN_FLOOR].input = &BUTTON_FLOOR_2_INPUT;
	floor_buttons[2-MIN_FLOOR].direction = &BUTTON_FLOOR_2_DIRECTION;
	floor_buttons[2-MIN_FLOOR].pin_offset = BUTTON_FLOOR_2_PIN;
	
	floor_buttons[3-MIN_FLOOR].port = &BUTTON_FLOOR_3_PORT;
	floor_buttons[3-MIN_FLOOR].input = &BUTTON_FLOOR_3_INPUT;
	floor_buttons[3-MIN_FLOOR].direction = &BUTTON_FLOOR_3_DIRECTION;
	floor_buttons[3-MIN_FLOOR].pin_offset = BUTTON_FLOOR_3_PIN;
	
	floor_buttons[4-MIN_FLOOR].port = &BUTTON_FLOOR_4_PORT;
	floor_buttons[4-MIN_FLOOR].input = &BUTTON_FLOOR_4_INPUT;
	floor_buttons[4-MIN_FLOOR].direction = &BUTTON_FLOOR_4_DIRECTION;
	floor_buttons[4-MIN_FLOOR].pin_offset = BUTTON_FLOOR_4_PIN;
	
	floor_buttons[5-MIN_FLOOR].port = &BUTTON_FLOOR_5_PORT;
	floor_buttons[5-MIN_FLOOR].input = &BUTTON_FLOOR_5_INPUT;
	floor_buttons[5-MIN_FLOOR].direction = &BUTTON_FLOOR_5_DIRECTION;
	floor_buttons[5-MIN_FLOOR].pin_offset = BUTTON_FLOOR_5_PIN;
	
	floor_leds[1-MIN_FLOOR].port = &LED_FLOOR_1_PORT;
	floor_leds[1-MIN_FLOOR].input = &LED_FLOOR_1_INPUT;
	floor_leds[1-MIN_FLOOR].direction = &LED_FLOOR_1_DIRECTION;
	floor_leds[1-MIN_FLOOR].pin_offset = LED_FLOOR_1_PIN;
	
	floor_leds[2-MIN_FLOOR].port = &LED_FLOOR_2_PORT;
	floor_leds[2-MIN_FLOOR].input = &LED_FLOOR_2_INPUT;
	floor_leds[2-MIN_FLOOR].direction = &LED_FLOOR_2_DIRECTION;
	floor_leds[2-MIN_FLOOR].pin_offset = LED_FLOOR_2_PIN;
	
	floor_leds[3-MIN_FLOOR].port = &LED_FLOOR_3_PORT;
	floor_leds[3-MIN_FLOOR].input = &LED_FLOOR_3_INPUT;
	floor_leds[3-MIN_FLOOR].direction = &LED_FLOOR_3_DIRECTION;
	floor_leds[3-MIN_FLOOR].pin_offset = LED_FLOOR_3_PIN;
	
	floor_leds[4-MIN_FLOOR].port = &LED_FLOOR_4_PORT;
	floor_leds[4-MIN_FLOOR].input = &LED_FLOOR_4_INPUT;
	floor_leds[4-MIN_FLOOR].direction = &LED_FLOOR_4_DIRECTION;
	floor_leds[4-MIN_FLOOR].pin_offset = LED_FLOOR_4_PIN;
	
	floor_leds[5-MIN_FLOOR].port = &LED_FLOOR_5_PORT;
	floor_leds[5-MIN_FLOOR].input = &LED_FLOOR_5_INPUT;
	floor_leds[5-MIN_FLOOR].direction = &LED_FLOOR_5_DIRECTION;
	floor_leds[5-MIN_FLOOR].pin_offset = LED_FLOOR_5_PIN;
	
	doors_led.port = &LED_DOORS_PORT;
	doors_led.input = &LED_DOORS_INPUT;
	doors_led.direction = &LED_DOORS_DIRECTION;
	doors_led.pin_offset = LED_DOORS_PIN;
	
	movement_led.port = &LED_MOVEMENT_PORT;
	movement_led.input = &LED_MOVEMENT_INPUT;
	movement_led.direction = &LED_MOVEMENT_DIRECTION;
	movement_led.pin_offset = LED_MOVEMENT_PIN;
}