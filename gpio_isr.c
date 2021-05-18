// @file gpio_isr.c
#include "gpio_isr.h"
#include "gpio_lab.h"

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[32];

void gpio0__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  // 2) Configure GPIO 0 pin for rising or falling edge

  gpio0_callbacks[pin] = callback; // store the function pointer into corresponding pin

  port_pin_s p = {.port = 0, .pin = pin};
  gpio_set_as_input(p); // make pin an input

  if (interrupt_type) {
    LPC_GPIOINT->IO0IntEnR |= (1 << pin); // enable rising edge
  } else {
    LPC_GPIOINT->IO0IntEnF |= (1 << pin); // enable falling edge
  }
}

// We wrote some of the implementation for you
void gpio0__interrupt_dispatcher(void) {

  // Check which pin generated the interrupt
  const int pin_that_generated_interrupt = get_pin_that_caused_interrupt();
  function_pointer_t attached_user_handler = gpio0_callbacks[pin_that_generated_interrupt];

  // Invoke the user registered callback, and then clear the interrupt
  attached_user_handler();
  clear_pin_interrupt(pin_that_generated_interrupt);
}

int get_pin_that_caused_interrupt(void) {
  unsigned i = 1, pos = 1;
  if (LPC_GPIOINT->IO0IntStatR == 0) {
    while (!(i & LPC_GPIOINT->IO0IntStatF)) {
      i = i << 1;
      ++pos;
    }
    pos--;
  } else if (LPC_GPIOINT->IO0IntStatF == 0) {
    while (!(i & LPC_GPIOINT->IO0IntStatR)) {
      i = i << 1;
      ++pos;
    }
    pos--;
  }
  return pos;
}

void clear_pin_interrupt(const int pin_to_clear) { LPC_GPIOINT->IO0IntClr |= (1 << pin_to_clear); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// // @file gpio_isr.c
// #include "gpio_isr.h"

// // Note: You may want another separate array for falling vs. rising edge callbacks
// static function_pointer_t gpio_callbacks[64];
// // static function_pointer_t gpio2_callbacks[32];

// void gpio__attach_interrupt(port_pin_s pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
//   // 1) Store the callback based on the pin at gpio0_callbacks
//   // 2) Configure GPIO 0 pin for rising or falling edge
//   if (pin.port == 0) {
//     gpio_callbacks[pin.pin] = callback; // store the function pointer into corresponding pin
//   } else if (pin.port == 2) {
//     gpio_callbacks[pin.pin + 32] = callback;
//   }

//   gpio_set_as_input(pin); // make pin an input

//   if (interrupt_type) {
//     if (pin.port == 0) {
//       LPC_GPIOINT->IO0IntEnR |= (1 << pin.pin); // enable rising edge
//     } else if (pin.port == 2) {
//       LPC_GPIOINT->IO2IntEnR |= (1 << pin.pin); // enable rising edge
//     }
//   } else {
//     if (pin.port == 0) {
//       LPC_GPIOINT->IO0IntEnF |= (1 << pin.pin); // enable falling edge
//     } else if (pin.port == 2) {
//       LPC_GPIOINT->IO2IntEnF |= (1 << pin.pin); // enable falling edge
//     }
//   }
// }

// // We wrote some of the implementation for you
// void gpio__interrupt_dispatcher(void) {

//   // Check which pin generated the interrupt
//   const int pin_that_generated_interrupt = get_pin_that_caused_interrupt();
//   function_pointer_t attached_user_handler = gpio_callbacks[pin_that_generated_interrupt];

//   // Invoke the user registered callback, and then clear the interrupt
//   attached_user_handler();
//   clear_pin_interrupt(pin_that_generated_interrupt);
// }

// int get_pin_that_caused_interrupt(void) {
//   unsigned i = 1, pos = 1;
//   if (LPC_GPIOINT->IO0IntStatR == 0) {
//     while (!(i & LPC_GPIOINT->IO0IntStatF)) {
//       i = i << 1;
//       ++pos;
//     }
//     pos--;
//   } else if (LPC_GPIOINT->IO0IntStatF == 0) {
//     while (!(i & LPC_GPIOINT->IO0IntStatR)) {
//       i = i << 1;
//       ++pos;
//     }
//     pos--;
//   }
//   if (LPC_GPIOINT->IO2IntStatR == 0) {
//     while (!(i & LPC_GPIOINT->IO2IntStatF)) {
//       i = i << 1;
//       ++pos;
//     }
//     pos--;
//     pos = pos + 32;
//   } else if (LPC_GPIOINT->IO2IntStatF == 0) {
//     while (!(i & LPC_GPIOINT->IO2IntStatR)) {
//       i = i << 1;
//       ++pos;
//     }
//     pos--;
//     pos = pos + 32;
//   }
//   return pos;
// }

// void clear_pin_interrupt(const int pin_to_clear) {
//   if (pin_to_clear < 32) {
//     LPC_GPIOINT->IO0IntClr |= (1 << pin_to_clear);
//   }
//   if (pin_to_clear >= 32) {
//     LPC_GPIOINT->IO2IntClr |= (1 << pin_to_clear - 32);
//   }
// }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
