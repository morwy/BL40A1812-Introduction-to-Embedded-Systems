#include "bit_ops.h"
#include "lcd.h"
#include "mcu.h"
#include "uart.h"
#include "timer0.h"
#include "adc.h"

#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

#define LCD_MAX_STRING (32)

static void handle_error(uint8_t return_code) {
    if (return_code){
        while(1);
    }
}

static void write_to_lcd(const char *string) {
    uint8_t len = strnlen(string, LCD_MAX_STRING);
    if (len == LCD_MAX_STRING) {
        printf("Failed to print LCD string. Too big or lacks NULL-terminator.\r\n");
        for (uint8_t i = 0; i < len; i++) {
            printf("%c", string[i]);
        }
        printf("\r\n");
        handle_error(1);
    } else {
        printf("LCD output: '%s'\r\n", string);
        lcd_puts(string);
    }
}

void setup(void) {
    // Initialize serial port for standard library
    uint8_t rc = setup_uart_io();
    handle_error(rc);
    printf("UART initialized.\r\n");

    CLEAR_BIT(DDRC, PC3); // Set A3 as input
    printf("Pin A3 set to input.\r\n");
    CLEAR_BIT(PORTC, PC3); // Disable pull-up on A3
    printf("Pin A3 pull-up disabled.\r\n");
    setup_timer0();
    printf("Timer0 initialized.\r\n");
    setup_adc();
    printf("ADC initialized.\r\n");
    set_sleep_mode(SLEEP_MODE_IDLE);
    printf("Sleep mode set to IDLE.\r\n");
    sei();
    printf("Global interrupts enabled.\r\n");
    lcd_init(LCD_DISP_ON);
    printf("LCD initialized.\r\n");
    lcd_clrscr();
    printf("LCD cleared.\r\n");
    write_to_lcd("Ready");
    _delay_ms(1000);
}

int main(void) {
    setup();

    uint16_t adc_result = 0;
    uint32_t previous_time = get_time() >> 6;
    uint32_t new_time = 0;
    bool new_result_ready = false;
    char buffer[16];

    while (1) {
        new_result_ready = try_reading_adc(&adc_result);
        if (new_result_ready) {
            snprintf(buffer, sizeof(buffer), "%d", adc_result);
            lcd_clrscr();
            write_to_lcd("The ADC result \n");
            write_to_lcd(buffer);
        }

        new_time = previous_time;
        while (new_time == previous_time) {
            sleep_mode();
            new_time = get_time() >> 6;
        }
        previous_time = new_time;
    }
}