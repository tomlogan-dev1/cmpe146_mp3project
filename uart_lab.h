#include <stdbool.h>
#include <stdint.h>

#include "lpc40xx.h"

typedef enum {
  UART_2,
  UART_3,
} uart_number_e;

void init_uart(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate);
bool uart_polled_recieve(uart_number_e uart, char *input_byte);
bool uart_polled_send(uart_number_e uart, char output_byte);

static void set_default_fdr(void);

static void set_data_length(void);

static void turn_uart_on(uart_number_e uart);

static void set_uart_pins(uart_number_e uart);

static void set_uart_baudrate(uint32_t baud_rate);

static void set_peripheral_clock(uint32_t peripheral_clock);

static void wait_for_end_of_transmission(void);

static void wait_for_byte(void);

// Private queue handle of our uart_lab.c
// static QueueHandle_t your_uart_rx_queue;

// Private function of our uart_lab.c
static void your_receive_interrupt(void);

void uart__enable_receive_interrupt(uart_number_e uart_number);

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout);