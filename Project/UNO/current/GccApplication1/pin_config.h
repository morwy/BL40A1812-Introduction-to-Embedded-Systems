#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <avr/io.h>

// --- UNO SLAVE PIN DEFINITIONS ---

// Buzzer (Arduino Pin D10)
#define BUZZER_PORT     PORTB
#define BUZZER_DDR      DDRB
#define BUZZER_PIN      PB2

// Obstacle LED (Arduino Pin D13)
#define OBSTACLE_PORT   PORTB
#define OBSTACLE_DDR    DDRB
#define OBSTACLE_PIN    PB5

// HC-SR04 Sensor (Arduino Pins D4 & D5)
#define TRIG_PORT       PORTD
#define TRIG_DDR        DDRD
#define TRIG_PIN        PD4
#define ECHO_PORT       PORTD
#define ECHO_DDR        DDRD
#define ECHO_PIN        PD5

#endif