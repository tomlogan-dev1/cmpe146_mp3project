#include "lpc40xx.h"
#include <stdio.h>

void init_spi2_pins(void);
void init_spi2_periferial(uint32_t max_clock_mhz);
void init_spi(void);
void turn_spi2_on(void);

void select_flash(void);
void unselect_flash(void);
void select_mp3(void);
void unselect_mp3(void);

void spi2_wait_until_done(void);
uint8_t spi2_exchange_byte(uint8_t output);
