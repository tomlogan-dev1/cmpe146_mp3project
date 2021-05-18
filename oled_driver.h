#pragma once

#include "lpc40xx.h"
#include "oled_fonts.h"
#include "periodic_scheduler.h"
#include "player_ani.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//*************************************//
// Define Commands from Command Table //
//*************************************//

// Fundemental Command Table //
#define RESET_DISPLAY 0x7F
#define SET_CONTRAST 0x81 // next byte is value from 1 -> 256
#define ENTIRE_DISPLAY_ON 0xA5
#define SET_NORMAL 0xA6
#define SET_INVERSE 0xA7
#define SET_DISPLAY_OFF 0xAE
#define SET_DISPLAY_ON 0xAF

// Scrolling Command Table //
#define RIGHT_SCROLL 0x26
#define LEFT_SCROLL 0x27
#define DEACTIVATE_SCROLL 0x2E // must reset RAM after this command
#define ACTIVATE_SCROLL 0x2F   // ex. write(0x26) then write(0x2F) to start right scroll

#define SET_MUX_RATIO 0xA8
#define SET_DISPLAY_OFFSET 0xD3
#define SET_START_LINE 0x40
#define SET_REMAP 0xA0
#define SET_SCAN_DIR 0xC0
#define SET_COM_PINS 0xDA
#define DISABLE_ALL_ON 0xA4
#define SET_FREQ 0xD5
#define EN_CHARGE_PUMP 0x8D
#define SET_MEM_ADDR_MODE 0x20

static void spi1__init_pins(void);
static void spi1__power_on_peripheral(void);
static void spi1__init_ctrl_registers(void);
static void spi1__init_clock_prescalar(uint32_t max_clock_mhz);
static void spi1__init_peripherals(uint32_t max_clock_mhz);
static uint8_t spi1__exchange_byte(uint8_t data_out);

static void enable_command_transfer(void);

static void disable_command_transfer(void);

static void enable_data_transfer(void);

static void disable_data_transfer(void);
void oled_init(void);
void loading_screen(void);

void spi1__init(uint32_t max_clock_mhz);
void whole_screen_on(void);
void clear_screen(void);
void clear_screen2(void);
void change_brightness(uint8_t brightness);
void display_buffer(void);
void edit_buffer(char c, int pos);
void display_song_name(char *name);
void display_text(char *text, int row);
void clear_buffer(void);
void move_cursor(int row);
void clear_buffer_except_cursor(void);