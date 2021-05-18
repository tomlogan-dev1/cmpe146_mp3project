#pragma once

#include "lpc40xx.h"
#include "song_list.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t current_pos; // current position of the cursor
  uint8_t range;       // the range of the cursor
  char *name;          // name of page
  char *text_data[64]; // limit of 30 different strings
  int top;
  uint8_t bottom;
  uint8_t cursor_pos;
} display_page;

typedef struct {
  uint8_t current_page;    // current position of the cursor
  uint8_t number_of_pages; // total pages
  display_page *page[8];   // collection of pages: home, settings, songs, current_song_playing
} page_set;

void display_current_page(display_page page);
void scroll_page(int cur, int dir, page_set *pages);
int get_current_pos(page_set pages);
void update_page(display_page *page, int direction);
void display_bass(uint8_t bass);
void display_treble(uint8_t treble);
void display_brightness(void);
void display_volume(uint8_t volume);
void display_song(display_page page);