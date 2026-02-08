#ifndef ADC_H
#define ADC_H

#include <stdbool.h>
#include <stdint.h>

/** Set up the ADC conversion.
These functions converts input voltage on A3 into 10 bit digital value (0-1023) */
void setup_adc(void);

/**
* This function is used to poll the latest ADC conversion result.
* @param uint16_t * result Pointer to location where to store the result of conversion
*
* return bool True if the result was ready and written to value pointed by result,
* false otherwise
* */
bool try_reading_adc(uint16_t *result);

#endif //ADC_H