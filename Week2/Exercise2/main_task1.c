#include "mcu.h"
#include "uart.h"

#include <avr/io.h>
#include <util/delay.h> /* for delay */

#include "bit_ops.h"
#include "board_config.h"
#include "task1_pin_config.h"

// / There is not much we can do for now . This function will be improved in future .
static void handle_error(uint8_t return_code)
{
    // Non - zero return code indicates critical fault
    if (return_code)
    {
        while (1)
            ;
    }
}

int main(void)
{
    // Initialize serial port for standard library
    uint8_t rc = setup_uart_io();
    handle_error(rc);
    printf("Configuring IO.\r\n");

    CLEAR_BIT(LED_8_PORT, LED_8_PIN);    // Default output value LOW
    SET_BIT(LED_8_DIRECTION, LED_8_PIN); // Set pin to OUTPUT
    _delay_ms(1000);
    CLEAR_BIT(LED_9_PORT, LED_9_PIN);    // Default output value LOW
    SET_BIT(LED_9_DIRECTION, LED_9_PIN); // Set pin to OUTPUT
    _delay_ms(1000);
    CLEAR_BIT(LED_10_PORT, LED_10_PIN);    // Default output value LOW
    SET_BIT(LED_10_DIRECTION, LED_10_PIN); // Set pin to OUTPUT
    _delay_ms(1000);
    CLEAR_BIT(LED_11_PORT, LED_11_PIN);    // Default output value LOW
    SET_BIT(LED_11_DIRECTION, LED_11_PIN); // Set pin to OUTPUT

    printf(" Configuration done.\r\n");
}