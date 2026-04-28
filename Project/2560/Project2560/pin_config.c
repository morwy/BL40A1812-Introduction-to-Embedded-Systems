#include "pin_config.h"

avr_gpio_t doors_led;
avr_gpio_t movement_led;

void init_avr_gpio_pins(void) {

	doors_led.port = &LED_DOORS_PORT;
	doors_led.input = &LED_DOORS_INPUT;
	doors_led.direction = &LED_DOORS_DIRECTION;
	doors_led.pin_offset = LED_DOORS_PIN;
	
	movement_led.port = &LED_MOVEMENT_PORT;
	movement_led.input = &LED_MOVEMENT_INPUT;
	movement_led.direction = &LED_MOVEMENT_DIRECTION;
	movement_led.pin_offset = LED_MOVEMENT_PIN;
}