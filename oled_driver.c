#include "oled_driver.h"
static const uint32_t oled_chip_select = (1 << 9);
static const uint32_t oled_dc = (1 << 7);

static uint8_t buffer[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  128, 192, 224, 240, 240, 112, 120,
    56,  60,  28,  28,  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,  28,  28, 60,  56,  120, 120, 240, 240, 224,
    192, 192, 128, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    128, 224, 240, 252, 62,  31,  15,  7,   3,   1,   0,   0,   0,   0,   0,   0,  224, 240, 240, 240, 248, 248, 248,
    248, 248, 252, 252, 252, 252, 252, 254, 254, 254, 0,   0,   1,   3,   7,   15, 31,  126, 248, 240, 192, 0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   240, 254, 255, 255, 7,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   255, 255, 255, 15,  15,  7,   7,   7,   7,   3,  3,   3,   3,   3,   255, 255, 255,
    0,   0,   0,   0,   0,   0,   0,   0,   1,   15,  255, 255, 248, 0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   15,  255, 255, 255, 128, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  128, 128, 128, 255, 255, 255, 0,
    0,   0,   0,   0,   128, 192, 224, 224, 240, 240, 255, 255, 255, 0,   0,   0,  0,   0,   0,   0,   0,   0,   128,
    255, 255, 127, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  7,   31,  63,  252, 240, 224, 192,
    0,   0,   60,  126, 127, 255, 255, 255, 127, 127, 63,  31,  0,   0,   0,   0,  0,   3,   15,  15,  31,  31,  31,
    15,  7,   3,   0,   0,   0,   0,   128, 192, 224, 248, 126, 63,  15,  3,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   3,   7,   15,  14,  30, 60,  56,  120, 112, 112, 240, 224,
    224, 224, 192, 192, 192, 192, 192, 192, 192, 224, 224, 224, 240, 112, 112, 56, 60,  28,  30,  15,  7,   3,   1,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,  1,   1,   1,   1,   1,   1,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

static void spi1__init_pins(void) {

  LPC_IOCON->P0_7 &= ~(0b111 << 0); // sck
  LPC_IOCON->P0_9 &= ~(0b111 << 0); // mosi
  LPC_IOCON->P2_9 &= ~(0b111 << 0); // cs
  LPC_IOCON->P2_7 &= ~(0b111 << 0); // dc
  LPC_IOCON->P2_4 &= ~(0b111 << 0); // rst

  // port & pin function set
  LPC_IOCON->P0_7 |= (0b010 << 0); // sck
  LPC_IOCON->P0_9 |= (0b010 << 0); // mosi
  LPC_IOCON->P2_9 |= (0b000 << 0); // cs   gpio
  LPC_IOCON->P2_7 |= (0b000 << 0); // dc   gpio
  LPC_IOCON->P2_4 |= (0b000 << 0); // rst

  // gpio port & pin direction
  LPC_GPIO2->DIR |= oled_chip_select; // cs  output
  LPC_GPIO2->DIR |= oled_dc;          // dc   output
  LPC_GPIO2->DIR |= (1 << 4);
}

static void spi1__power_on_peripheral(void) {
  const uint32_t power_on_SSP1 = (1 << 10); // table 16
  LPC_SC->PCONP |= power_on_SSP1;
}

static void spi1__init_ctrl_registers(void) {
  uint8_t spi_enable = (0b1 << 1);
  uint8_t bit8_data_transfer = (0b0111 << 0);
  uint8_t spi_frame_formate = (0b00 << 4);
  LPC_SSP1->CR0 |= bit8_data_transfer | spi_frame_formate;
  LPC_SSP1->CR1 |= spi_enable;
}

static void spi1__init_clock_prescalar(uint32_t max_clock_mhz) {
  uint8_t divider = 2;
  const uint32_t cpu_clock_mhz = clock__get_core_clock_hz();
  while (max_clock_mhz < (cpu_clock_mhz / divider) && divider <= 254) {
    divider += 2;
  }
  LPC_SSP1->CPSR = divider;
}

static void spi1__init_peripherals(uint32_t max_clock_mhz) {
  spi1__power_on_peripheral();
  spi1__init_ctrl_registers();
  spi1__init_clock_prescalar(max_clock_mhz);
}

static void spi1_busy_wait(void) {
  const uint32_t spi_busy = (1 << 4);
  while (LPC_SSP1->SR & spi_busy) {
    ;
  }
}

static uint8_t spi1__exchange_byte(uint8_t data_out) {
  LPC_SSP1->DR = data_out;
  spi1_busy_wait();
  return LPC_SSP1->DR;
}

static void enable_command_transfer(void) {
  // chip select low
  // D/#C low
  LPC_GPIO2->CLR = oled_chip_select;
  LPC_GPIO2->CLR = oled_dc;
}

static void disable_command_transfer(void) {
  // chip select high
  // D/#C high
  LPC_GPIO2->SET = oled_chip_select;
  LPC_GPIO2->SET = oled_dc;
}

static void enable_data_transfer(void) {
  // chip select low
  // D/#C high
  LPC_GPIO2->SET = oled_dc;
  LPC_GPIO2->CLR = oled_chip_select;
}

static void disable_data_transfer(void) {
  // chip select high
  // D/#C low
  LPC_GPIO2->SET = oled_chip_select;
  LPC_GPIO2->SET = oled_dc;
}

void spi1__init(uint32_t max_clock_mhz) {
  spi1__init_pins();
  spi1__init_peripherals(max_clock_mhz);
}

void clear_screen2(void) {
  enable_command_transfer();
  (void)spi1__exchange_byte(0x20);
  disable_command_transfer();
  enable_command_transfer();
  (void)spi1__exchange_byte(0x02);
  disable_command_transfer();

  for (int page = 0; page < 8; page++) {
    enable_command_transfer();
    (void)spi1__exchange_byte(0x40);
    disable_command_transfer();
    enable_command_transfer();
    (void)spi1__exchange_byte(0xB0 + page);
    disable_command_transfer();
    for (int higher_col = 0; higher_col < 8; higher_col++) {
      enable_command_transfer();
      (void)spi1__exchange_byte(0x10 + higher_col);
      disable_command_transfer();
      for (int lower_col = 0; lower_col < 16; lower_col++) {
        enable_command_transfer();
        (void)spi1__exchange_byte(0x00 + lower_col);
        disable_command_transfer();
        enable_data_transfer();
        (void)spi1__exchange_byte(0x00);
        disable_data_transfer();
      }
    }
  }
}

void clear_screen(void) {
  enable_command_transfer();
  (void)spi1__exchange_byte(0xAE);
  (void)spi1__exchange_byte(0x20);
  (void)spi1__exchange_byte(0x00);
  (void)spi1__exchange_byte(0xA4);
  (void)spi1__exchange_byte(0xAF);
  (void)spi1__exchange_byte(0x22);
  (void)spi1__exchange_byte(0x00);
  (void)spi1__exchange_byte(0x07);
  (void)spi1__exchange_byte(0x21);
  (void)spi1__exchange_byte(0x00);
  (void)spi1__exchange_byte(127);

  disable_command_transfer();
  enable_data_transfer();
  for (int i = 0; i < 1024; i++) {
    (void)spi1__exchange_byte(0);
  }
  disable_data_transfer();
}

void display_buffer(void) {
  enable_command_transfer();
  //(void)spi1__exchange_byte(0xAE); // display off
  (void)spi1__exchange_byte(0x20); // horizontal addressing
  (void)spi1__exchange_byte(0x00);
  //(void)spi1__exchange_byte(0xA4); // display on
  //(void)spi1__exchange_byte(0xAF); // display on
  (void)spi1__exchange_byte(0x22);
  (void)spi1__exchange_byte(0x00);
  (void)spi1__exchange_byte(0x07);
  (void)spi1__exchange_byte(0x21);
  (void)spi1__exchange_byte(0x00);
  (void)spi1__exchange_byte(127);
  disable_command_transfer();
  // edit_buffer(0xff, 896);
  enable_data_transfer();
  for (int i = 0; i < 1024; i++) {
    (void)spi1__exchange_byte(buffer[i]);
  }
  disable_data_transfer();
}

// void clear_buffer(void){
//   for(int i=0; i < 1024; i++){
//     buffer[i] = 0;
//   }
// }

void oled_init(void) {
  enable_command_transfer();

  (void)spi1__exchange_byte(0xAE); // Set display OFF

  (void)spi1__exchange_byte(0xD5); // Set Display Clock Divide Ratio / OSC Frequency
  (void)spi1__exchange_byte(0x80); // Display Clock Divide Ratio / OSC Frequency

  (void)spi1__exchange_byte(0xA8); // Set Multiplex Ratio
  (void)spi1__exchange_byte(0x3F); // Multiplex Ratio for 128x64 (64-1)

  (void)spi1__exchange_byte(0xD3); // Set Display Offset
  (void)spi1__exchange_byte(0x00); // Display Offset

  (void)spi1__exchange_byte(0x40); // Set Display Start Line

  (void)spi1__exchange_byte(0x8D); // Set Charge Pump
  (void)spi1__exchange_byte(0x14); // Charge Pump (0x10 External, 0x14 Internal DC/DC)

  (void)spi1__exchange_byte(0xA1); // Set Segment Re-Map
  (void)spi1__exchange_byte(0xC8); // Set Com Output Scan Direction

  (void)spi1__exchange_byte(0xDA); // Set COM Hardware Configuration
  (void)spi1__exchange_byte(0x12); // COM Hardware Configuration

  (void)spi1__exchange_byte(0x81); // Set Contrast
  (void)spi1__exchange_byte(0xFF); // Contrast

  (void)spi1__exchange_byte(0xD9); // Set Pre-Charge Period
  (void)spi1__exchange_byte(0xF1); // Set Pre-Charge Period (0x22 External, 0xF1 Internal)

  (void)spi1__exchange_byte(0xDB); // Set VCOMH Deselect Level
  (void)spi1__exchange_byte(0x40); // VCOMH Deselect Level

  (void)spi1__exchange_byte(0xA4); // Set all pixels OFF
  (void)spi1__exchange_byte(0xA6); // Set display not inverted
  LPC_GPIO2->CLR = (1 << 4);
  delay__ms(5);
  LPC_GPIO2->SET = (1 << 4);
  // clear screen
  clear_screen();

  (void)spi1__exchange_byte(0xAF); // Set display On
  disable_command_transfer();
}

void whole_screen_on(void) {
  enable_command_transfer();
  (void)spi1__exchange_byte(0xA5);
  disable_command_transfer();
}

void change_brightness(uint8_t brightness) {
  enable_command_transfer();
  (void)spi1__exchange_byte(0x81);       // Set Contrast
  (void)spi1__exchange_byte(brightness); // Contrast
  disable_command_transfer();
}

void display_song_name(char *name) {
  size_t length = strlen(name);
  int current_pos = 896;
  for (int i = 896; i < 1024; i++) {
    edit_buffer(0x00, i);
  }
  for (int j = 0; j < length - 4; j++) { // -4 to remove .mp3
    fprintf(stderr, "%c", name[j]);
    int ascii = name[j] - 32;
    fprintf(stderr, "%d", ascii);
    for (int i = 0; i < 6; i++) {
      edit_buffer(lower_case_6x8[(ascii * 6) + i], current_pos);
      current_pos++;
    }
  }
  display_buffer();
}

void display_text(char *text, int row) {
  int current_pos = row * 128 + 8;
  size_t length = strlen(text);
  if (length > 20) {
    length = 20;
  }
  for (int i = 0; i < length; i++) {
    int ascii = text[i] - 32;
    for (int j = 0; j < 6; j++) {
      edit_buffer(lower_case_6x8[(ascii * 6) + j], current_pos);
      current_pos++;
    }
  }
}

void move_cursor(int row) {
  for (int i = 0; i < 8; i++) {
    if (i == row) {
      for (int j = 0; j < 3; j++) {
        edit_buffer(0xff, 128 * row + j);
      }
    } else {
      for (int j = 0; j < 3; j++) {
        edit_buffer(0x00, 128 * i + j);
      }
    }
  }
  // display_buffer();
}

void edit_buffer(char c, int pos) { buffer[pos] = c; }

void play_animation(int frame) {
  for (int i = 0; i < 896; i++) {
    buffer[i] = animation[frame][i];
  }
  display_buffer();
}

void clear_buffer(void) {
  for (int i = 0; i < 1024; i++) {
    buffer[i] = 0;
  }
}

void clear_buffer_except_cursor(void) {
  for (int i = 0; i < 8; i++) {
    int current_pos = 128 * i + 8;
    for (int j = current_pos; j < 128; j++) {
      edit_buffer(0, j);
    }
  }
}

// void add_char_to_buffer(char c){

// }
