#define F_CPU 16000000UL
#define DELAY 100
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "lcd.h"

float time_from_start = 0; //to count time from start
volatile float counter = 0; //to count how many interrupt that has been done.

ISR(INT0_vect){
    counter = counter + 1; //means increase the counter value by one
}

static void handle_error(uint8_t return_code) {
    if (return_code){
        while(1);
    }
}

int main(void){
    DDRD &= ~(1 << PD2) & ~(1 << PD7);
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
    SMCR |= (1 << SM1);
    sei();
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();

    while (1) {
        char count[16];
        dtostrf(counter, 3, 0, count); // Convert counter to string
        
        if ((PIND & (1 << PD7)) != 0) { // Check if PD7 is high
            lcd_clrscr(); // Clear LCD screen
            lcd_puts(count); // Display counter value on LCD
            SMCR |= (1 << SE); // Enable sleep mode
            sleep_cpu(); // Enter sleep mode
            SMCR &= ~(1 << SE); // Disable sleep mode after waking up
        }
        else {
            time_from_start = time_from_start + 1; // Increment time from start
            char time_elapsed[16];
            dtostrf(time_from_start/(float)1000/DELAY, 3, 2, time_elapsed); // Convert time from start to string
            lcd_clrscr(); // Clear LCD screen
            lcd_puts(time_elapsed); // Display time elapsed on LCD
        }
    } 
    return 0;
}