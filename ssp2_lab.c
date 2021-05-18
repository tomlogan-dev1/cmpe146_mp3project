#include "ssp2_lab.h"

const uint32_t chip_select_pin = (1 << 10);
const uint32_t mp3_select_pin = (1 << 0);

void init_spi2_pins(void) {
  char clear = 0b111;
  char spi_func = 0b100;

  // prepare pins to be initialized into their spi functionality //
  LPC_IOCON->P1_0 &= ~clear;
  LPC_IOCON->P1_1 &= ~clear;
  LPC_IOCON->P1_4 &= ~clear;
  LPC_IOCON->P1_10 &= ~clear;

  LPC_IOCON->P1_0 |= spi_func;       // Port 1, Pin 0 -> SCK2
  LPC_IOCON->P1_1 |= spi_func;       // Port 1, Pin 1 -> MOSI2
  LPC_IOCON->P1_4 |= spi_func;       // Port 1, Pin 4 -> MISO2
  LPC_GPIO1->DIR |= chip_select_pin; // Port 1, Pin 10 -> Chip Select
  LPC_GPIO2->DIR |= mp3_select_pin;  // Port 2, Pin 0
}

void turn_spi2_on(void) {
  const uint32_t ssp2_periferial = (1 << 20); // table 16.
  LPC_SC->PCONP |= ssp2_periferial;
}

void init_spi2_periferial(uint32_t max_clock_mhz) {
  turn_spi2_on();

  LPC_SSP2->CR0 = (0b111 << 0) | (0b00 << 4); // 8-bit transfer size
  // LPC_SSP2->CR0 = (0b00 << 4);  // Frame Format: SPI

  LPC_SSP2->CR1 = (0b1 << 1); // enable SSP controller

  uint8_t prescalar = 96 / max_clock_mhz;
  LPC_SSP2->CPSR = prescalar; // 96/4 = 24Mhz speed for controller
}

void select_flash(void) { LPC_GPIO1->CLR = chip_select_pin; }

void unselect_flash(void) { LPC_GPIO1->SET = chip_select_pin; }

void select_mp3(void) { LPC_GPIO2->CLR = mp3_select_pin; }

void unselect_mp3(void) { LPC_GPIO2->SET = mp3_select_pin; }

void init_spi(void) {
  init_spi2_pins();
  init_spi2_periferial(24);
  // unselect_flash();
}

void spi2_wait_until_done(void) {
  const uint32_t busy = (1 << 4);
  while (LPC_SSP2->SR & busy) {
    ;
  }
}

uint8_t spi2_exchange_byte(uint8_t output) {
  LPC_SSP2->DR = output;
  spi2_wait_until_done();
  // fprintf(stderr, "COMMS UP");
  //   uint8_t info = LPC_SSP2->DR;
  //   printf("0x%02X\n", info);
  return LPC_SSP2->DR;
}
