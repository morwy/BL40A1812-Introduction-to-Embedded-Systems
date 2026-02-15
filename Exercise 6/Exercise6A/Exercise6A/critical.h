#ifndef CRITICAL_H
#define CRITICAL_H

#include <avr/interrupt.h>
#include <stdio.h>

/// This global keeps track of nested critical sections.
extern uint32_t critical_nesting;

/// Enter critical code section which prevents interrupts until it is exited
static void enter_critical(void) {
   // First disable the interrupts
   cli();
   // Then increment nesting counter to only enable interrupts back when we exit all 
   // nested levels
   critical_nesting++;
}

/// Exit critical code section
static void exit_critical(void) {
   // If we are not in critical section log warning as this should not happen.
   if (!critical_nesting) {
	printf("Exiting critical section that we have not entered!");
	return;
   } else {
   	critical_nesting--;
	// If we reached 0 it is time to enable interrupts again.
	if (!critical_nesting) {
	    sei();
	}
   }
}

#endif
