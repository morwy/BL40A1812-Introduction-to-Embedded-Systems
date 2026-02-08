#include "delay.h"
#include "lcd.h"
#include "mcu.h"
#include "uart.h"
#include "stdutils.h"
#include "keypad.h"

#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


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

    // Initialize keypad
    KEYPAD_Init();
    printf("Keypad initialized.\r\n");

    // Initialize LCD and clear display
    lcd_init(LCD_DISP_ON);
    printf("LCD initialized.\r\n");
    lcd_clrscr();
    printf("LCD cleared.\r\n");

    //Write "Ready" to LCD
    write_to_lcd("Ready");
}

int main(void) {
    static char key_str[32];
    setup();

    uint32_t memory = 0;

    while (1) {
        printf("Waiting for key press...\r\n");
        uint8_t key = KEYPAD_GetKey();  // Wait for a key press
        printf("Key pressed: '%c'\r\n", key);

        uint8_t key_value = key - '0'; // Convert ASCII to integer value
        
        if (key_value <= 9) { // If key is a digit
            if (memory <= (UINT32_MAX / 10 - key_value)) { // Check for overflow
                memory = memory * 10 + key_value;
            }
        } else if (key == '*') {
            memory = 0;
        }
        
        lcd_clrscr(); // Clear LCD before displaying new info
        snprintf(key_str, sizeof(key_str), "Key pressed: %c", key);
        write_to_lcd(key_str);

        write_to_lcd("\n"); // New line on LCD

        snprintf(key_str, sizeof(key_str), "Mem: %"PRIu32"", memory);
        write_to_lcd(key_str);

    }
}