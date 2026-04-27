#include "buzzer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

const uint16_t melody_notes[] = {NOTE_E5, NOTE_D5, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_E5, NOTE_E5};
const uint16_t melody_durations[] = {300, 300, 300, 300, 300, 300, 600};
const uint8_t total_notes = 7;

volatile uint32_t system_millis = 0; 
uint32_t note_start_time = 0;
uint8_t current_note_index = 0;
bool is_playing = false;

ISR(TIMER0_COMPA_vect) {
	system_millis++;
}

//Pitch Generator 
ISR(TIMER1_COMPA_vect) {
	BUZZER_PORT ^= (1 << BUZZER_PIN); // Toggle pin state
}

//Set Frequency
static void set_buzzer_frequency(uint16_t freq) {
	if (freq == NOTE_REST) {
		
		TIMSK1 &= ~(1 << OCIE1A);
		BUZZER_PORT &= ~(1 << BUZZER_PIN); 
		return;
	}

	// Math to calculate Timer1 compare value for desired frequency
	// Formula: OCR1A = (CPU_FREQ / (2 * PRESCALER * FREQ)) - 1
	// prescaler 8
	uint16_t ocr_value = (16000000UL / (16UL * freq)) - 1;
	
	OCR1A = ocr_value;
	TIMSK1 |= (1 << OCIE1A); 
}

// --- INITIALIZATION ---
void buzzer_init(void) {
	BUZZER_DDR |= (1 << BUZZER_PIN);
	BUZZER_PORT &= ~(1 << BUZZER_PIN);

	// 2. Configure Timer 0 (The Stopwatch - 1ms ticks)
	TCCR0A = (1 << WGM01);
	TCCR0B = (1 << CS01) | (1 << CS00);
	OCR0A = 249; // 16MHz / 64 / 1000Hz - 1 = 249
	TIMSK0 |= (1 << OCIE0A); // Enable Timer0 compare interrupt

	// 3. Configure Timer 1 (The Pitch Generator)
	// CTC Mode (WGM12=1), Prescaler 8 (CS11=1)
	TCCR1B = (1 << WGM12) | (1 << CS11);
}

// --- PUBLIC CONTROLS ---
void buzzer_start_melody(void) {
	if (!is_playing) {
		is_playing = true;
		current_note_index = 0;
		note_start_time = system_millis;
		set_buzzer_frequency(melody_notes[0]);
	}
}

void buzzer_stop_melody(void) {
	is_playing = false;
	set_buzzer_frequency(NOTE_REST); // Silence
}

void buzzer_update(void) {
	if (!is_playing) return; 
	
	uint32_t current_time = system_millis;
	if ((current_time - note_start_time) >= melody_durations[current_note_index]) {
		
		current_note_index++;

		if (current_note_index >= total_notes) {
			current_note_index = 0; // Loop the melody
		}

		set_buzzer_frequency(melody_notes[current_note_index]);
		note_start_time = current_time;
	}
}