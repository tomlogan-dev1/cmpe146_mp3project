#pragma once
#include <stdint.h>

void spi0__init(uint32_t max_clock_mhz);

void spi_send_data_to_mp3_decoder(uint8_t song_data);

void mp3_set_volume(uint16_t volume);

void mp3_write_register(uint8_t address_byte, uint16_t data);

uint16_t mp3_read_register(uint8_t address_byte);

void mp3_set_bass(uint16_t bass);

void mp3_set_treble(uint16_t treble);
