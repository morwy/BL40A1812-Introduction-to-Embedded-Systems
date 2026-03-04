#define F_CPU 16000000UL
#define DELAY 100
#include <avr/sleep.h>
#include <util/delay.h>
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
    return 0;
}