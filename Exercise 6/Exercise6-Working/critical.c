#include <stdint.h>

// This global keeps track of nested critical sections.
uint32_t critical_nesting = 0;