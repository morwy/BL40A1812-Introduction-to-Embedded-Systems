#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "board_config_2560.h"
#include "avr_gpio.h"

/// GPIO pin mappings from Arduino MEGA pins

// Movement & Doors LEDs mappings:

#define LED_DOORS_PORT IO_32_PORT
#define LED_DOORS_DIRECTION IO_32_DIRECTION
#define LED_DOORS_INPUT IO_32_INPUT
#define LED_DOORS_PIN IO_32_PIN

#define LED_MOVEMENT_PORT IO_31_PORT
#define LED_MOVEMENT_DIRECTION IO_31_DIRECTION
#define LED_MOVEMENT_INPUT IO_31_INPUT
#define LED_MOVEMENT_PIN IO_31_PIN

/////////////////////////////////

// Changed as in the project description it must be able to use floor 0
#define FLOOR_NOT_SELECTED (-1)
#define MIN_FLOOR (0)
#define MAX_FLOOR (99)

void init_avr_gpio_pins(void);

extern avr_gpio_t doors_led;
extern avr_gpio_t movement_led;

#endif
