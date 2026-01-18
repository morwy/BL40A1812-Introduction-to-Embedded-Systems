#include "mcu.h"
#include "uart.h"

# include <avr/io.h>
# include <util/delay.h> /* for delay */

# include "bit_ops.h"
# include "board_config.h"
# include "task2_pin_config.h"


static void handle_error(uint8_t return_code) {
    if (return_code){
        while(1);
    }
}

int main(void){

    // Initialize serial port for standard library
    uint8_t rc = setup_uart_io();
    handle_error(rc);

    //clear buttons to work as input
    CLEAR_BIT(BUTTON_1_DIRECTION, BUTTON_1_PIN);
    CLEAR_BIT(BUTTON_1_DIRECTION, BUTTON_1_PIN);

    //set bit low, direction output
    CLEAR_BIT(LED_PORT, LED_PIN);
    SET_BIT(LED_DIRECTION, LED_PIN);

    uint8_t switch_button_1 = 0; /* unsigned integer 8 bit wide */
    uint8_t switch_button_2 = 0;

    while(1){
        printf("Loop reading inputs.\r\n");
        switch_button_1 = READ_BIT(BUTTON_1_INPUT, BUTTON_1_PIN);
        switch_button_2 = READ_BIT(BUTTON_2_INPUT, BUTTON_2_PIN);

        printf("Loop SWITCH 1: %d SWITCH 2: %d.\r\n ", switch_button_1, switch_button_2);

        if ( switch_button_1 && switch_button_2 )
        {
            printf("LED is ON.\r\n");
            SET_BIT(LED_PORT, LED_PIN);
        }
        else
        {
            printf("LED is OFF.\r\n");
            CLEAR_BIT(LED_PORT, LED_PIN);
        }
        _delay_ms(1000);
    }


}