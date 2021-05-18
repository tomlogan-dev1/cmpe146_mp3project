
#include "uart_lab.h"
#include "FreeRTOS.h"
#include <stdio.h>

#include "lpc_peripherals.h"
#include "queue.h"

// Private queue handle of our uart_lab.c
static QueueHandle_t your_uart_rx_queue;

void init_uart(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {

  turn_uart_on(uart);
  set_peripheral_clock(peripheral_clock);
  set_uart_baudrate(baud_rate);
  set_default_fdr();
  set_data_length();
  set_uart_pins(uart);
}

static void set_default_fdr(void) {
  const uint32_t default_reset_fdr_value = (1 << 4);
  LPC_UART2->FDR = default_reset_fdr_value;
}

static void set_data_length(void) {
  const uint32_t eight_bit = (3 << 0);
  LPC_UART2->LCR |= eight_bit;
}

static void turn_uart_on(uart_number_e uart) {
  const uint32_t uart2_on = (1 << 24);
  LPC_SC->PCONP |= uart2_on;
}

static void set_uart_pins(uart_number_e uart) {
  // p0_10 U2_Tx 001 ->2.8
  // p0_11 U2_Rx 001 ->2.9

  const uint32_t uart_func = (1 << 1); // 010
  LPC_IOCON->P2_8 |= uart_func;
  LPC_IOCON->P2_9 |= uart_func;
}

static void set_uart_baudrate(uint32_t baud_rate) {
  const uint32_t DLAB = (1 << 7);
  // DLAB must be 1 to set the baudrate
  LPC_UART2->LCR = DLAB;
  const uint16_t divider_16_bit = 96 * 1000 * 1000 / (16 * baud_rate);
  LPC_UART2->DLM = (divider_16_bit >> 8) & 0xFF;
  LPC_UART2->DLL = (divider_16_bit >> 0) & 0xFF;
  LPC_UART2->LCR &= ~DLAB;
}

static void set_peripheral_clock(uint32_t peripheral_clock) {
  const uint32_t clk_prescalar = peripheral_clock / 96;
  // const uint32_t clk_prescalar = (1<<0); // divide by 1
  LPC_SC->PCLKSEL |= clk_prescalar;
}

bool uart_polled_recieve(uart_number_e uart, char *input_byte) {
  wait_for_byte();
  *input_byte = LPC_UART2->RBR & 0xFF;
  return true;
}

bool uart_polled_send(uart_number_e uart, char output_byte) {
  wait_for_end_of_transmission();
  LPC_UART2->THR = output_byte;
  wait_for_end_of_transmission();
  // fprintf(stderr, "BYTE SENT\n");
  return true;
}

static void wait_for_byte(void) {
  const uint32_t char_available = (1 << 0);
  while (!(LPC_UART2->LSR & char_available)) {
    // fprintf(stderr, "waiting for byte\n");
  }
}

static void wait_for_end_of_transmission(void) {
  const uint32_t transmitter_empty = (1 << 5);
  while (!(LPC_UART2->LSR & transmitter_empty)) {
    // fprintf(stderr, "waiting to end transmission\n");
  }
}

// Private function of our uart_lab.c
static void your_receive_interrupt(void) {

  while (LPC_UART2->IIR & (1 << 0)) {
    ; // wait for interrupt
  }

  while (LPC_UART2->IIR & (1 << 2)) {
    if (LPC_UART2->LSR & (1 << 0)) {
      const char recived_byte = LPC_UART2->RBR & 0xFF;
      xQueueSendFromISR(your_uart_rx_queue, &recived_byte, NULL);
    }
  }

  // if (!(LPC_UART2->IIR & (1 << 0)) && LPC_UART2->IIR & (2 << 1)) {
  //   const uint32_t char_available = (1 << 0);
  //   while (!(LPC_UART2->LSR & char_available)) {
  //     // fprintf(stderr, "waiting for byte\n");
  //   }
  //   const char recived_byte = LPC_UART2->RBR;
  //   xQueueSendFromISR(your_uart_rx_queue, &recived_byte, NULL);
  // }
}

void uart__enable_receive_interrupt(uart_number_e uart_number) {

  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, your_receive_interrupt, "uart interrupt");
  NVIC_EnableIRQ(UART2_IRQn);
  LPC_UART2->IER |= (1 << 0); // RBR
  LPC_UART2->IER |= (1 << 0); // THR

  your_uart_rx_queue = xQueueCreate(30, sizeof(char));
}

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}