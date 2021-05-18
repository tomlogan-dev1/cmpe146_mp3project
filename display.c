#include "display.h"
#include "oled_driver.h"
#include "player_ani.h"

void display_song(display_page page) {
  char *song = page.text_data[page.current_pos];
  clear_buffer();
  display_text(song, 7);
  for (int i = 0; i < 896; i++) {
    edit_buffer(frame1[i], i);
  }
  display_buffer();
}

void display_bass(uint8_t bass) {
  clear_buffer();
  display_text("bass:", 1);
  bass = bass * 4;
  // for (int i = 40; i < 15 * 4 + 40; i++) {
  //   edit_buffer(0x00, i);
  //   edit_buffer(0x00, i + 128);
  // }
  for (int i = 40; i < bass + 40; i++) {
    edit_buffer(0xff, i);
    edit_buffer(0xff, i + 128);
  }
  display_buffer();
}

void display_treble(uint8_t treble) {
  clear_buffer();
  display_text("treble:", 1);
  if (treble < 8) {
    treble = treble * 4;
    for (int i = 80; i < treble + 80; i++) {
      edit_buffer(0xff, i);
      edit_buffer(0xff, i + 128);
    }
  } else if (treble >= 8) {
    treble = (16 - treble) * 4;
    for (int i = 80; i > 80 - treble; i--) {
      edit_buffer(0xff, i);
      edit_buffer(0xff, i + 128);
    }
  }
  display_buffer();
}

void display_brightness(void) {
  clear_buffer();
  display_text("     brightness", 1);
  for (int i = 290; i < 360; i++) {
    edit_buffer(0xff, i);
    edit_buffer(0xff, i + 128);
    edit_buffer(0xff, i + 256);
    edit_buffer(0xff, i + 384);
  }
  display_buffer();
}

void display_volume(uint8_t volume) {
  clear_buffer();
  display_text("vol:", 1);
  int range = 58 - volume / 2;

  for (int i = 32; i < range + 32; i++) {
    edit_buffer(0xff, i);

    edit_buffer(0xff, i + 128);
  }
  display_buffer();
  fprintf(stderr, "vol %d\n", volume);
}

void display_current_page(display_page page) {
  clear_buffer();
  for (int i = 0; i < 8; i++) {
    display_text(page.text_data[page.top + i], i);
  }
  move_cursor(page.cursor_pos);
  display_buffer();
}

void scroll_page(int cur, int dir, page_set *pages) {
  int cur_pos = get_current_pos(*pages);
  if (cur_pos > 0 && dir == -1) {
    pages->page[pages->current_page]->current_pos--;
  }
  if (cur_pos < pages->page[pages->current_page]->range && dir == 1) {
    pages->page[pages->current_page]->current_pos++;
  }
  fprintf(stderr, "current position in page: %d\n", cur_pos);
  //   if (cur > 0 && cur < 7) { // no need to scroll the page
  //     return;
  //   }
  //   if (cur == 0 && dir == -1 && (cur_pos > 0)) {
  //     pages->page[pages->current_page]->current_pos--;
  //   } else if (cur == 7 && dir == 1 && (cur_pos >= 7) && (pages->page[pages->current_page]->range > cur_pos)) {
  //     pages->page[pages->current_page]->current_pos++;
  //   }
  // display_current_page(*pages);
}

int get_current_pos(page_set pages) { return pages.page[pages.current_page]->current_pos; }

void update_page(display_page *page, int direction) {
  if (direction == 1) {
    if (page->cursor_pos < 7) {
      page->cursor_pos++;
    }
    if ((page->range - 1) < 7 && page->cursor_pos > (page->range - 1)) {
      page->cursor_pos = (page->range - 1);
    }
    if (page->current_pos < page->range - 1) {
      page->current_pos++;
    }
    if (page->current_pos > page->bottom) {
      page->bottom = page->current_pos;
      page->top = page->bottom - 7;
    }
  }
  if (direction == -1) {
    if (page->cursor_pos > 0) {
      page->cursor_pos--;
    }
    if (page->current_pos > 0) {
      page->current_pos--;
    }
    if (page->current_pos < page->top) {
      page->top = page->current_pos;
      page->bottom = page->top + 7;
    }
  }
  display_current_page(*page);
  fprintf(stderr, "cursor pos %d\n", page->cursor_pos);
  fprintf(stderr, "current pos %d\n", page->current_pos);

  // fprintf(stderr, "cursor pos %d\n", cursor_pos);
}