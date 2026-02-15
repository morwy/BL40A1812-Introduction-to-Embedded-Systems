#ifndef CRITICAL_H
#define CRITICAL_H
#include <avr/interrupt.h>
#include <stdio.h>

/// This global keeps track of nested critical sections.
extern uint32_t critical_nesting;

/// Enter critical code section which prevents interrupts until it is exited
static void enter_critical(void) {
    // First disable the interrupts with cli()
    cli();
    // Then increment nesting counter to only enable interrupts back when we exit all
    // nested levels
    critical_nesting++;
}

/// Exit critical code section
static void exit_critical(void) {
    // If we are not in critical section log warning as this should not happen.
    if (critical_nesting == 0) {
        fprintf(stderr, "Warning: Attempting to exit critical section when not in one.\n");
        return;
    }
    // Otherwise decrement the nesting.
    // If we reached 0 it is time to enable interrupts again with sei()
    critical_nesting--;
    if (critical_nesting == 0) {
        sei();
    }
}

#endif