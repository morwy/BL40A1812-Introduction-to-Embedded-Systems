#ifndef HCSR04_H
#define HCSR04_H

#include <stdint.h>
#include <stdbool.h>
#include "pin_config.h"
#include "i2c_protocol.h"

// Define how close an object must be to trigger the obstacle state (in cm)
#define OBSTACLE_THRESHOLD_CM 10

// Function Prototypes
void hcsr04_init(void);
void hcsr04_update(void);

#endif