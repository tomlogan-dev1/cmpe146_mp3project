// @file gpio_isr.h
#include "gpio_lab.h"
#include "lpc40xx.h"
#pragma once

typedef enum {
  GPIO_INTR__FALLING_EDGE,
  GPIO_INTR__RISING_EDGE,
} gpio_interrupt_e;

// Function pointer type (demonstrated later in the code sample)
typedef void (*function_pointer_t)(void);

// Allow the user to attach their callbacks
void gpio0__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback);

// Our main() should configure interrupts to invoke this dispatcher where we will invoke user attached callbacks
// You can hijack 'interrupt_vector_table.c' or use API at lpc_peripherals.h
void gpio0__interrupt_dispatcher(void);
int get_pin_that_caused_interrupt(void);
void clear_pin_interrupt(const int pin_to_clear);

void gpio2__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback);

// Our main() should configure interrupts to invoke this dispatcher where we will invoke user attached callbacks
// You can hijack 'interrupt_vector_table.c' or use API at lpc_peripherals.h
void gpio2__interrupt_dispatcher(void);
int get_pin_that_caused_interrupt2(void);
void clear_pin_interrupt2(const int pin_to_clear);

////////////////////////////////////////////////////////

// // Function pointer type (demonstrated later in the code sample)
// typedef void (*function_pointer_t)(void);

// // Allow the user to attach their callbacks
// void gpio__attach_interrupt(port_pin_s pin, gpio_interrupt_e interrupt_type, function_pointer_t callback);

// // Our main() should configure interrupts to invoke this dispatcher where we will invoke user attached callbacks
// // You can hijack 'interrupt_vector_table.c' or use API at lpc_peripherals.h
// void gpio__interrupt_dispatcher(void);
// int get_pin_that_caused_interrupt(void);
// void clear_pin_interrupt(const int pin_to_clear);
