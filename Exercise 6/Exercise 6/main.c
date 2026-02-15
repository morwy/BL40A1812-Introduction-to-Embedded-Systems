#include "mcu.h"
#include "uart.h"
#include "board_config_2560.h"
#include "timer0.h"
#include "timer1.h"
#include "tune.h"
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

note_t test_tune_1[] = {
    { 3, HALF },
    { 15, HALF },
    { 50, HALF },
    { 100, HALF },
    { 150, HALF },
    { 300, HALF },
    { 400, HALF },
    { 600, HALF },
    { 600, HALF }};

note_t test_tune_2[] = {
    { C3, QUARTER },
    { E3, QUARTER },
    { G3, QUARTER },
    { C3, QUARTER },
    { E3, QUARTER },
    { G3, QUARTER },
    { E3, QUARTER },
    { G3, QUARTER },
    { B3, QUARTER },
    { E3, QUARTER },
    { G3, QUARTER },
    { B3, QUARTER },
    { A3, QUARTER },
    { F3, QUARTER },
    { D3, QUARTER },
    { A3, QUARTER },
    { F3, QUARTER }
};

static void handle_error(uint8_t return_code) {
    CLEAR_BIT(IO_5_PORT, IO_5_PIN);
    SET_BIT(IO_5_DIRECTION, IO_5_PIN);
    // Non-zero return code indicates critical fault
    if (return_code){
        while(1) {
            // Long blink, then short blinks based on the return_code
            // Repeat to keep indicating fault
            _delay_ms(1000);
            CLEAR_BIT(IO_5_PORT, IO_5_PIN);
            _delay_ms(200);
            for (uint8_t i = 0; i < return_code; i++) {
                SET_BIT(IO_5_PORT, IO_5_PIN);
                _delay_ms(200);
                CLEAR_BIT(IO_5_PORT, IO_5_PIN);
                _delay_ms(200);
            }
        }
    }
    // Short blink as confirmation for everything working fine.
    SET_BIT(IO_5_PORT, IO_5_PIN);
    _delay_ms(100);
    CLEAR_BIT(IO_5_PORT, IO_5_PIN);
}

void setup(void){
    // Initialize serial port for standard library
	printf("Setting up the serial port...\r\n");
	uint8_t rc = setup_uart_io();
	handle_error(rc);

    // Set Port C pin 11 as input with default output low.
	DDRC &= ~(1 << DDC11);	 // Clear bit 11 to set as input
	PORTC &= ~(1 << PORTC11); // Clear bit 11 to 

    // Call setup function of the Timer 0
	printf("Setting up Timer 0...\r\n");
	setup_timer0();

    // Configure sleep mode of the processor set_sleep_mode(SLEEP_MODE_IDLE);
	printf("Configuring sleep mode...\r\n");
	set_sleep_mode(SLEEP_MODE_IDLE);

    // Enable interrupts globally 
    sei();

    // Write ’Ready’ to serialport
    printf("Ready\r\n");

    	// Busy-wait 1 second
	printf("Waiting for 1 second...\r\n");
	_delay_ms(1000);

    printf("Done.\r\n");
}

static void sleep_ms(uint32_t time) {
    uint32_t end = get_time() + time/10;
    while (get_time() < end) {
        sleep_mode();
    }
}

int main(void) {
    setup();

    uint8_t current_note = 0;

    while (1) {

        if (current_note < sizeof(test_tune_1)/sizeof(note_t)){

            uint16_t playingNote = test_tune_1[current_note];
            uint16_t frequenzyToUse = test_tune_1[current_note].frequency_hz;
            uint16_t noteDuration = test_tune_1[current_note].duration_ms;

            printf("Playing: %d", playingNote);

            timer_1_set_frequenzy(frequenzyToUse);

            timer1_channel_A_on();

            sleep_ms(noteDuration);

            timer1_channel_A_off();

            current_note = current_note + 1;
        } else {
            printf("Tune played. \r\n");
            sleep_ms(1000);
            current_note = 0;
        }

    }
}