#include "hcsr04.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h> // Only for the mandatory 10us trigger pulse

// --- STATE VARIABLES ---
volatile uint32_t timer2_overflows = 0;
volatile uint32_t echo_start_time = 0;
volatile uint32_t echo_duration = 0;
volatile bool new_distance_ready = false;

uint32_t last_trigger_time = 0;

extern volatile uint8_t obstacle_status;

// --- TIMER 2 OVERFLOW ISR (The Microsecond Stopwatch) ---
// With prescaler 64, this overflows every 1.024 milliseconds
ISR(TIMER2_OVF_vect) {
	timer2_overflows++;
}

// Combine the overflow and current timer value
uint32_t get_micros(void) {
	uint32_t overflows;
	uint8_t tcnt2_val;
	
	
	//Briefly disable interrupts for safe reading
	cli();
	overflows = timer2_overflows;
	tcnt2_val = TCNT2;
	sei();
	
	// Prescaler 64 at 16MHz = 4 microseconds per tick
	return (overflows * 256 + tcnt2_val) * 4;
}

// Detect Echo 
ISR(PCINT2_vect) {
	
	bool is_echo_high = (PIND & (1 << ECHO_PIN));
	
	if (is_echo_high) {
		// RISING EDGE: The sound wave left. Start the clock.
		echo_start_time = get_micros();
		} else {
		// FALLING EDGE: The sound wave return. Stop the clock.
		echo_duration = get_micros() - echo_start_time;
		new_distance_ready = true;
	}
}


void hcsr04_init(void) {
	// 1. Configure Pins
	TRIG_DDR |= (1 << TRIG_PIN);  // TRIG = Output
	ECHO_DDR &= ~(1 << ECHO_PIN); // ECHO = Input
	TRIG_PORT &= ~(1 << TRIG_PIN); // Set TRIG low initially

	// 2. Configure Timer 2 (Microsecond Stopwatch)
	// Normal mode, Prescaler 64
	TCCR2A = 0x00;
	TCCR2B = (1 << CS22);
	TIMSK2 |= (1 << TOIE2); 

	// 3. Configure Pin Change Interrupts for ECHO pin (PD5)
	PCICR |= (1 << PCIE2);    // Enable Pin Change Interrupts for PORTD
	PCMSK2 |= (1 << PCINT21); // Unmask PCINT21 (which corresponds to PD5)
}

void hcsr04_update(void) {
	uint32_t current_time = get_micros();

	// 1. Trigger the sensor every 60ms
	// wait between pings to reduce overlap chance pings
	if ((current_time - last_trigger_time) > 60000) {
		// Send a 10 microsecond pulse.
		// Note: _delay_us(10) is perfectly fine. It is only 160 clock cycles
		TRIG_PORT |= (1 << TRIG_PIN);
		_delay_us(10);
		TRIG_PORT &= ~(1 << TRIG_PIN);
		
		last_trigger_time = current_time;
	}

	// 2. If the interrupt caught a new echo, calculate the distance
	if (new_distance_ready) {
		new_distance_ready = false; // Reset flag

		// Calculate distance in cm (Duration / 58)
		uint32_t distance_cm = echo_duration / 58;

		// Update the global status variable that the I2C bus reads
		if (distance_cm > 0 && distance_cm < OBSTACLE_THRESHOLD_CM) {
			obstacle_status = STATUS_OBSTACLE;
			} else {
			obstacle_status = STATUS_CLEAR;
		}
	}
}