#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include <stdbool.h>

// Standard Note Frequencies (in Hertz)
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_REST 0 // Silence

// Define the Buzzer Pin (From your pin_config.h / Schematic)
#define BUZZER_PORT PORTB
#define BUZZER_DDR  DDRB
#define BUZZER_PIN  PB2

// Function Prototypes
void buzzer_init(void);
void buzzer_start_melody(void);
void buzzer_stop_melody(void);
void buzzer_update(void); // The non-blocking state machine

#endif