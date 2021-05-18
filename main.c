#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "lpc40xx.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

#include "gpio_lab.h"
#include "semphr.h"

#include "lpc_peripherals.h"

#include "gpio_isr.h"

// #include "adc.h"
// #include "pwm1.h"

#include "queue.h"

//#include "ssp2_lab.h"

//#include "uart_lab.h"

// #include "sj2_cli.h"

// #include "app_cli.h"
// #include "cli__your"

#include "sl_string.h"

#include "event_groups.h"

#include "ff.h"
#include <string.h>

#include "oled_driver.h"

#include "song_list.h"

#include "display.h"
#include "spi_mp3.h"

#if 1
////////////
static song_memory_t list_of_songs[32];
char *my_songs[32];
static size_t number_of_songs;
#define DATA_TRANSFER_SIZE 1024
#define SONG_NAME_SIZE 32
char song_name[SONG_NAME_SIZE];

#define CURRENT_SONG_PLAYING_ 3
#define HOME_ 0
#define SETTINGS_ 2
#define SONGS_ 1
#define BASS_ 4
#define TREBLE_ 5
#define BRIGHTNESS_ 6
#define VOLUME_ 7
#define MAX_VOLUME 0
#define MIN_VOLUME 65000

QueueHandle_t Q_songname;
QueueHandle_t Q_songdata;

volatile bool playing = false;
volatile bool next_song = false;
volatile bool prev_song = false;

volatile int current_song = 0;
volatile int timer_count = 0;
volatile int current_frame = 0;
volatile int position = 0;
volatile int cursor_pos = 0;
volatile int brightness = 255;
volatile int8_t volume = 30;
volatile uint16_t bass_treble = 0x0108;
volatile bool rotated = false;
volatile bool rotated2 = false;
volatile bool back_button = false;
volatile bool change_vol = false;
volatile bool change_b = false;

volatile TickType_t last_interrupt_time = 0;
const int frame_rate = 100;
static SemaphoreHandle_t play_pause_button;
static SemaphoreHandle_t next_song_button;
static SemaphoreHandle_t change_song_request;
static SemaphoreHandle_t update_page_request;
static SemaphoreHandle_t display_volume_request;
static SemaphoreHandle_t stop_volume_display;
static SemaphoreHandle_t return_current_song_request;
static SemaphoreHandle_t get_current_time_request;

static void mp3_reader_task(void *p);
static void mp3_player_task(void *p);
static void control_task(void *p);
static void timer_task(void *p);
static void return_to_current_song_task(void *p);

void play_pause_pressed(void);
void next_song_pressed(void);
void previous_song_pressed(void);
int song_direction(void);
void next_song_pressed(void);
void previous_song_pressed(void);
void rotation_isr(void);
void rotation_isr2(void);
void select_button_isr(void);
void back_button_isr(void);
volatile bool select_button = false;
void button_clicked(page_set *page);
void back_button_clicked(page_set *page);
void change_song(int dir);
void volume_isr(void);
void change_volume(int dir);
void change_treble(int dir);
void change_bass(int dir);
void adjust_brightness(int dir);

static bool mp3_decoder_needs_data(void);

static SemaphoreHandle_t spi_bus_mutex;

volatile TickType_t last_interaction;
volatile TickType_t current_time;
volatile TickType_t time_waited;

static void song_list__handle_filename(const char *filename) {
  // This will not work for cases like "file.mp3.zip"
  if (NULL != strstr(filename, ".mp3")) {
    printf("Filename: %s\n", filename);
    strncpy(list_of_songs[number_of_songs], filename, sizeof(song_memory_t) - 1);
    ++number_of_songs;
    // or
    // number_of_songs++;
  }
}

void song_list__populate(void) {
  FRESULT res;
  static FILINFO file_info;
  const char *root_path = "/";

  DIR dir;
  res = f_opendir(&dir, root_path);

  if (res == FR_OK) {
    for (;;) {
      res = f_readdir(&dir, &file_info); /* Read a directory item */
      if (res != FR_OK || file_info.fname[0] == 0) {
        break; /* Break on error or end of dir */
      }

      if (file_info.fattrib & AM_DIR) {
        /* Skip nested directories, only focus on MP3 songs at the root */
      } else { /* It is a file. */
        if (file_info.fsize > 10000) {
          song_list__handle_filename(file_info.fname);
        }
      }
    }
    f_closedir(&dir);
  }
}

size_t song_list__get_item_count(void) { return number_of_songs; }

const char *song_list__get_name_for_item(size_t item_number) {
  const char *return_pointer = "";

  if (item_number >= number_of_songs) {
    return_pointer = "";
  } else {
    return_pointer = list_of_songs[item_number];
  }

  return return_pointer;
}
////////////
port_pin_s rotary_pin = {.port = 1, .pin = 23};
port_pin_s rotary_pin0 = {.port = 0, .pin = 6};
port_pin_s pin0 = {.port = 0, .pin = 25};

display_page songs = {.current_pos = 0,
                      .name = 'songs',
                      .range = 20,
                      .text_data = {"song0",  "song1",  "song2",  "song3",  "song4",  "song5",  "song6",
                                    "song7",  "song8",  "song9",  "song10", "song11", "song12", "song13",
                                    "song14", "song15", "song16", "song17", "song18", "song19"},
                      .top = 0,
                      .bottom = 7};
display_page home = {
    .current_pos = 0, .name = 'home', .range = 2, .text_data = {"settings", "songs"}, .top = 0, .bottom = 7};
display_page settings = {.current_pos = 0,
                         .name = 'settings',
                         .range = 3,
                         .text_data = {"treble", "bass", "brightness"},
                         .top = 0,
                         .bottom = 7};
display_page current_song_playing = {
    .current_pos = 0, .name = 'current song', .range = 1, .text_data = {""}, .top = 0, .bottom = 7};
display_page bass_control = {
    .current_pos = 0, .name = "bass control", .range = 1, .text_data = {"bass: "}, .top = 0, .bottom = 7};
display_page treble_control = {
    .current_pos = 0, .name = "treble control", .range = 1, .text_data = {"treble: "}, .top = 0, .bottom = 7};
display_page brightness_control = {
    .current_pos = 0, .name = "brightness control", .range = 1, .text_data = {"brightness: "}, .top = 0, .bottom = 7};
display_page volume_control = {
    .current_pos = 0, .name = "volume control", .range = 1, .text_data = {"volume: "}, .top = 0, .bottom = 7};

page_set all_pages = {.current_page = 0,
                      .number_of_pages = 1,
                      .page = {&home, &songs, &settings, &current_song_playing, &bass_control, &treble_control,
                               &brightness_control, &volume_control}};
int main(void) {
  spi_bus_mutex = xSemaphoreCreateMutex();
  update_page_request = xSemaphoreCreateMutex();
  LPC_GPIO1->DIR &= ~(1 << rotary_pin.pin);
  spi1__init(3);
  oled_init();
  clear_screen();
  display_buffer();
  song_list__populate();
  for (size_t song_number = 0; song_number < song_list__get_item_count(); song_number++) {
    printf("Song %2d: %s\n", (1 + song_number), song_list__get_name_for_item(song_number));
  }
  printf("number of songs %2d", number_of_songs);
  for (int i = 0; i < sizeof(list_of_songs) / sizeof(list_of_songs[0]); i++) {
    char *name = list_of_songs[i];
    songs.text_data[i] = name;
    size_t length = strlen(name);
    for (int j = 0; j < length; j++) {
      fprintf(stderr, "%c", name[j]);
      int temp = name[j];
      fprintf(stderr, "%d", temp);
    }

    fprintf(stderr, "\n");
  }
  songs.range = number_of_songs;

  const uint32_t kilobyte = 1024;
  uint32_t max_clock_mhz = 1; // CLKI/7. 12.288/7 = 1.755Mhz Max Clock

  Q_songname = xQueueCreate(1, sizeof(song_name));
  Q_songdata = xQueueCreate(2, DATA_TRANSFER_SIZE);

  change_song_request = xSemaphoreCreateBinary();
  display_volume_request = xSemaphoreCreateBinary();
  stop_volume_display = xSemaphoreCreateBinary();
  return_current_song_request = xSemaphoreCreateBinary();
  get_current_time_request = xSemaphoreCreateBinary();

  spi0__init(max_clock_mhz);

  xTaskCreate(mp3_reader_task, "reader", (4 * kilobyte) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(mp3_player_task, "player", (4 * kilobyte) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(control_task, "controller", (10 * kilobyte) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(timer_task, "timer task", (2 * kilobyte) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  NVIC_EnableIRQ(GPIO_IRQn); // ??
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "unused");

  gpio0__attach_interrupt(6, GPIO_INTR__RISING_EDGE, rotation_isr);
  gpio0__attach_interrupt(25, GPIO_INTR__FALLING_EDGE, volume_isr);
  gpio0__attach_interrupt(8, GPIO_INTR__FALLING_EDGE, select_button_isr);
  gpio0__attach_interrupt(26, GPIO_INTR__FALLING_EDGE, back_button_isr);

  clear_buffer();
  update_page(all_pages.page[all_pages.current_page], 0);

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  while (1) {
    // fprintf(stderr, "pos: %d\n", position);
  }
  return 0;
}

static void timer_task(void *p) {
  while (1) {

    timer_count++;
    xSemaphoreGive(return_current_song_request);
    if (playing && timer_count > 6) {
      all_pages.current_page = CURRENT_SONG_PLAYING_;
      display_song(*all_pages.page[all_pages.current_page]);
      change_vol = false;
    }
    vTaskDelay(1000);
  }
}

static void control_task(void *p) {
  change_volume(0);
  int rotation_dir = 0;
  vTaskDelay(20);
  cursor_pos = 0;

  while (1) {
    vTaskDelay(1);
    if (rotated) {
      vTaskDelay(1);
      timer_count = 0;
      if (gpio_get_level(rotary_pin) == gpio_get_level(rotary_pin0)) {
        rotation_dir = 1;
      } else {
        rotation_dir = -1;
      }
      if (change_vol) {
        fprintf(stderr, "changing volume\n");
        change_volume(rotation_dir);
        display_volume(volume);
      } else if (all_pages.current_page == CURRENT_SONG_PLAYING_) {
        change_song(rotation_dir);
        display_song(*all_pages.page[all_pages.current_page]);
      } else if (all_pages.current_page == BASS_) {
        change_bass(rotation_dir);
        // display_bass(15);
      } else if (all_pages.current_page == TREBLE_) {
        change_treble(rotation_dir);
      } else if (all_pages.current_page == BRIGHTNESS_) {
        adjust_brightness(rotation_dir);
      } else {
        update_page(all_pages.page[all_pages.current_page], rotation_dir);
      }
      rotated = false;
    }
    if (select_button) {
      timer_count = 0;
      xSemaphoreGive(get_current_time_request);
      fprintf(stderr, "I was pressed\n");
      button_clicked(&all_pages);
      select_button = false;
    }
    if (back_button) {
      timer_count = 0;
      xSemaphoreGive(get_current_time_request);
      back_button_clicked(&all_pages);
      back_button = false;
    }
    if (xSemaphoreTakeFromISR(display_volume_request, 100)) {
      timer_count = 0;
      display_volume(volume);
    }
    if (xSemaphoreTakeFromISR(stop_volume_display, 100)) {
      timer_count = 0;
      update_page(all_pages.page[all_pages.current_page], 0);
    }
  }
}

static void mp3_reader_task(void *p) {
  char song_data[DATA_TRANSFER_SIZE];
  FIL file;    /* File objects */
  UINT br = 0; /* File read/write count */

  while (true) {
    if (playing) {
      char *song_name = list_of_songs[current_song];
      fprintf(stderr, "opening_song");
      FRESULT result = f_open(&file, song_name, FA_READ);

      if (FR_OK == result) {
        fprintf(stdout, "File opened: %s\n", song_name);
        // display_song_name(song_name);
        while (!f_eof(&file)) {

          if (xSemaphoreTake(change_song_request, 10)) {
            xQueueReset(Q_songdata);
            break;
          }
          f_read(&file, song_data, sizeof(song_data), &br);
          if (br == 0) {
            current_song++;
            break; // error or eof
          }
          if (xQueueSend(Q_songdata, &song_data[0], portMAX_DELAY)) {
            // fprintf(stderr, "sending data%d", song_data[0]);
          }
          if (f_eof(&file)) {
            change_song(1);
            display_song(*all_pages.page[all_pages.current_page]);
            // update_page(all_pages.page[all_pages.current_page], 0);
            break; // error or eof
          }
        }

        f_close(&file);
        printf("File closed: %s\n", song_name);
      } else {
        fprintf(stderr, "ERROR: Failed to open file: %s\n", song_name);
      }
    }
  }
}

static void mp3_player_task(void *p) {
  char song_data[DATA_TRANSFER_SIZE];

  while (true) {
    int count = 0;
    if (xQueueReceive(Q_songdata, &song_data[0], portMAX_DELAY)) {
      for (int i = 0; i < sizeof(song_data); i++) {
        count++;
        while (!mp3_decoder_needs_data()) {
          ;
        }
        if (playing) {
          if (xSemaphoreTake(spi_bus_mutex, 100)) {
            spi_send_data_to_mp3_decoder(song_data[i]);
            xSemaphoreGive(spi_bus_mutex);
          }

          // printf(stderr, count);

        } else {
          vTaskDelay(10);
        }
      }
    }
  }
}

void back_button_clicked(page_set *page) {
  fprintf(stderr, "back button\n");
  int cp = page->current_page;
  if (cp == SONGS_ || cp == SETTINGS_) {
    all_pages.current_page = HOME_;
    update_page(all_pages.page[all_pages.current_page], 0);
  } else if (cp == CURRENT_SONG_PLAYING_) {
    all_pages.current_page = SONGS_;
    update_page(all_pages.page[all_pages.current_page], 0);
  } else if (cp == BASS_ || cp == TREBLE_ || cp == BRIGHTNESS_) {
    all_pages.current_page = SETTINGS_;
    update_page(all_pages.page[all_pages.current_page], 0);
  }
  if (change_vol) {
    change_vol = false;
    update_page(all_pages.page[all_pages.current_page], 0);
  }
}

void button_clicked(page_set *page) {
  int cp = page->current_page;
  int cur_pos = all_pages.page[all_pages.current_page]->current_pos;
  if (change_vol) {
    return;
  }
  if (cp == HOME_) {
    if (cur_pos == 1) {
      all_pages.current_page = SONGS_; // songs is page 1
      fprintf(stderr, "entered song list");
    } else if (cur_pos == 0) {
      all_pages.current_page = SETTINGS_; // settings is page 2
    }
    update_page(all_pages.page[all_pages.current_page], 0);
  } else if (cp == SONGS_) {
    char *song = page->page[page->current_page]->text_data[cur_pos];
    if (current_song != cur_pos) {
      current_song = cur_pos;
      xSemaphoreGive(change_song_request);
    }
    playing = true;
    all_pages.current_page = 3; // page 3 is current playing song page
    page->page[page->current_page]->text_data[0] = song;
    fprintf(stderr, "entered current song \n");
    // printf("Excellent!\n");
    display_song(*all_pages.page[all_pages.current_page]);
    // update_page(all_pages.page[all_pages.current_page], 0);
  } else if (cp == SETTINGS_) {
    if (cur_pos == 1) {
      all_pages.current_page = BASS_;
      // update_page(all_pages.page[all_pages.current_page], 0);
      display_bass((bass_treble & 0x00f0) >> 4);
    } else if (cur_pos == 0) {
      all_pages.current_page = TREBLE_;
      display_treble((bass_treble & 0xf000) >> 12);
    } else if (cur_pos == 2) {
      all_pages.current_page = BRIGHTNESS_;
      display_brightness();
    }

  } else if (cp == CURRENT_SONG_PLAYING_) {
    if (playing) {
      playing = false;
    } else {
      playing = true;
    }
  }
}

bool song_control(void) {}

int song_direction(void) {
  if (next_song == prev_song) {
    next_song = false;
    prev_song = false;
    return 0; // both pressed, do not go anywhere. Or neither were pressed
  } else if (next_song) {
    next_song = false;
    return 1; // go into the positive direction
  } else if (prev_song) {
    prev_song = false;
    return -1; // go into the negative direction
  } else {
    return 0;
  }
}

void play_pause_pressed(void) {
  if (playing) {
    playing = false;
  } else {
    playing = true;
  }
}

void next_song_pressed(void) {
  // xSemaphoreGiveFromISR(next_song_button, NULL);
  next_song = true;
}
void previous_song_pressed(void) {
  // xSemaphoreGiveFromISR(previous_song_button, NULL);
  prev_song = true;
}

void change_song(int dir) {
  current_song = current_song + dir;
  if (current_song > number_of_songs - 1) {
    current_song = 0;
  }
  if (current_song < 0) {
    current_song = number_of_songs - 1;
  }
  current_song_playing.text_data[0] = list_of_songs[current_song];
  xSemaphoreGive(change_song_request);
}

void rotation_isr(void) { rotated = true; }
void rotation_isr2(void) { rotated2 = true; }
void select_button_isr(void) { select_button = true; }
void back_button_isr(void) { back_button = true; }
void volume_isr(void) {
  if (change_vol) {
    xSemaphoreGiveFromISR(stop_volume_display, NULL);
    change_vol = false;
  } else {
    xSemaphoreGiveFromISR(display_volume_request, NULL);
    change_vol = true;
  }
}

void change_volume(int dir) {
  // it seems backwards because MAX volume is actually 0
  uint16_t right_left;
  if (dir == 1) {
    volume = volume - 8;
    if (volume <= 0) {
      volume = 0;
    }
    right_left = (volume << 8) | (volume & 0xff);
  } else if (dir == -1) {
    volume = volume + 8;
    if (volume > 115) {
      volume = 115;
    }
  }
  right_left = (volume << 8) | (volume & 0xff);
  if (xSemaphoreTake(spi_bus_mutex, 100)) {
    mp3_set_volume(right_left);
    xSemaphoreGive(spi_bus_mutex);
  }
  if (xSemaphoreTake(spi_bus_mutex, 100)) {
    uint16_t mp3_mode = mp3_read_register(0x00);
    uint16_t mp3_status = mp3_read_register(0x01);
    uint16_t mp3_clock = mp3_read_register(0x03);
    uint16_t mp3_volume = mp3_read_register(0x0B);
    fprintf(stderr, "Mode:0x%04x Status:0x%04x Clock:0x%04x Vol:0x%04x\n", mp3_mode, mp3_status, mp3_clock, mp3_volume);
    xSemaphoreGive(spi_bus_mutex);
  }
}

void change_treble(int dir) {
  uint8_t treble = (bass_treble & 0xf000) >> 12;
  if (treble < 8) {
    if (dir == 1) {
      if (treble > 6) {
        treble = 7;
      } else {
        treble++;
      }
    } else if (dir == -1) {
      if (treble == 0) {
        treble = 15;
      } else {
        treble--;
      }
    }
  } else if (treble >= 8) {
    if (dir == 1) {
      if (treble == 15) {
        treble = 0;
      } else {
        treble++;
      }
    } else if (dir == -1) {
      if (treble < 9) {
        treble = 8;
      } else {
        treble--;
      }
    }
  }
  bass_treble = (bass_treble & 0x0FFF) | (treble << 12);
  if (xSemaphoreTake(spi_bus_mutex, 100)) {
    mp3_set_bass(bass_treble);
    uint16_t mp3_info = mp3_read_register(0x02);
    uint8_t t = (mp3_info & 0xf000) >> 12;
    fprintf(stderr, "info:0x%04x x\n", mp3_info);
    fprintf(stderr, "treble  %d\n ", t);
    xSemaphoreGive(spi_bus_mutex);
  }
  display_treble(treble);
}

void change_bass(int dir) {
  uint8_t bass = (bass_treble & 0x00f0) >> 4;
  if (dir == 1) {
    if (bass > 14) {
      bass = 15;
    } else {
      bass++;
    }
  } else if (dir == -1) {
    if (bass < 1) {
      bass = 0;
    } else {
      bass--;
    }
  }
  bass_treble = (bass_treble & 0xFF0F) | (bass << 4);
  if (xSemaphoreTake(spi_bus_mutex, 100)) {
    mp3_set_bass(bass_treble);
    uint16_t mp3_info = mp3_read_register(0x02);
    fprintf(stderr, "info:0x%04x x\n", mp3_info);
    xSemaphoreGive(spi_bus_mutex);
  }
  display_bass(bass);
}

void adjust_brightness(int dir) {
  if (dir == 1) {
    brightness = brightness + 25;
    if (brightness > 254) {
      brightness = 255;
    }
  } else if (dir == -1) {
    brightness = brightness - 25;
    if (brightness < 1) {
      brightness = 0;
    }
  }
  change_brightness(brightness);
  fprintf(stderr, "brightness %d \n", brightness);
  display_brightness();
}

#endif

#if 0


static QueueHandle_t mp3_data_block;
typedef char file_buffer_t[512];
static port_pin_s mp3_drequest = {.port = 2, .pin = 2};

static bool mp3_needs_data(void) {
  if (gpio_get_level(mp3_drequest)) {
    return true;
  } else {
    return false;
  }
}

static void send_to_decoder(const char c) {
  select_mp3();
  spi2_exchange_byte(c);
  unselect_mp3();
}

static void file_reader(void *p) {
  file_buffer_t file_buffer;
  UINT bytesRead;
  while (1) {
    const char *filename = "song.mp3";
    FIL file; // File handle
    FRESULT result = f_open(&file, filename, FA_READ);
    while (!f_eof(&file)) {
      f_read(&file, file_buffer, sizeof(file_buffer_t), &bytesRead);
      xQueueSend(mp3_data_block, &file_buffer, portMAX_DELAY);
    }
    f_close(&file);
  }
}

static void player(void *p) {
  file_buffer_t file_buffer;
  while (1) {
    xQueueReceive(mp3_data_block, &file_buffer, portMAX_DELAY);
    for (int i = 0; i < sizeof(file_buffer_t);) {
      if (mp3_needs_data()) {
        send_to_decoder(file_buffer[i]);
        fprintf(stderr, file_buffer[i]);
        i++;
      } else {
        vTaskDelay(1);
      }
    }
  }
}

int main(void) {
  init_spi();
  gpio_set_as_input(mp3_drequest);

  mp3_data_block = xQueueCreate(2, sizeof(file_buffer_t));
  xTaskCreate(file_reader, "reader", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(player, "player", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}

#endif

#if 0

#include "i2c.h"
// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);

//===part 3a===================
// i2c__2 taken as master, this function enable i2c__1 as slave
void i2c1__slave_init(uint8_t slave_address_to_respond_to) {
  LPC_I2C1->ADR0 = slave_address_to_respond_to; // Write address
  LPC_I2C1->CONSET = 0x44;
  // lpc_i2c->CONSET = 0x44;
}
//========part 3b =================
static volatile uint8_t slave_memory[256];
bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory) {
  // implement
  if (memory_index >= 256) // Exceed bounds
    return false;
  else {
    *memory = slave_memory[memory_index];
    return true;
  }
}

bool i2c_slave_callback__write_memory(uint8_t memory_index, uint8_t memory_value) {
  // implement
  if (memory_index >= 256) // Exceed bounds
    return false;
  else {
    slave_memory[memory_index] = memory_value;
    return true;
  }
}

void loop_task(void *p) {
  LPC_GPIO2->DIR |= (1 << 3);  // led0 
  LPC_GPIO1->DIR |= (1 << 26); // led1
  LPC_GPIO1->DIR |= (1 << 24); // led2
  LPC_GPIO1->DIR |= (1 << 18); // led3 ... all outputs

  while (1) {
    printf("In main\n");
    vTaskDelay(1000);

    // task displays the first 4 bits of the slave's memory on the leds
    const uint8_t memory = slave_memory[0];
    if (memory & 1){
      LPC_GPIO2->PIN |= (1 << 3);  // led0 high
    }else{
      LPC_GPIO2->PIN &= ~(1 << 3); // led0 low
    }
    if ((memory >> 1) & 1){
      LPC_GPIO1->PIN |= (1 << 26);  // led1 high
    }else{
      LPC_GPIO1->PIN &= ~(1 << 26); // led1 low
    }
    if ((memory >> 2) & 1){
      LPC_GPIO1->PIN |= (1 << 24);  // led2 high
    }else{
      LPC_GPIO1->PIN &= ~(1 << 24); // led2 low
    }
    if ((memory >> 3) &1){
      LPC_GPIO1->PIN |= (1 << 18);  // led3 high
    }else{
      LPC_GPIO1->PIN &= ~(1 << 18); // led3 low
    }
  }
}

void main(void) {
  create_uart_task();

  // setup pins for i2c__1
  // 1. Setup function as i2c pin
  LPC_IOCON->P0_0 &= ~(0b111);
  LPC_IOCON->P0_0 |= (0b011);
  LPC_IOCON->P0_1 &= ~(0b111);
  LPC_IOCON->P0_1 |= (0b011);
  // 2. Enable pin as open drain (open collector)
  uint16_t open_drain_mask = (1 << 10); // bit 10
  LPC_IOCON->P0_0 |= open_drain_mask;
  LPC_IOCON->P0_1 |= open_drain_mask;

  // initalize i2c__1
  i2c__initialize(I2C__1, UINT32_C(400) * 1000, clock__get_peripheral_clock_hz()); // setup i2c__1 as master
  i2c1__slave_init(0xFE); // Any number | This function overides i2c__1 to function as slave
  xTaskCreate(loop_task, "i2c_loop", 510, NULL, 1, NULL);

  vTaskStartScheduler();
}

static void create_uart_task(void) {
  // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
  // Change '#if (0)' to '#if (1)' and vice versa to try it out
#if (0)
  // printf() takes more stack space, size this tasks' stack higher
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#else
  sj2_cli__init();
  UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
#endif
}

// This sends periodic messages over printf() which uses system_calls.c to send them to UART0
static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
     *  - During debugging in case system crashes before all output of printf() is sent
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}

#endif

#if 0

#define PRODUCER_BIT (1 << 0)
#define CONSUMER_BIT (1 << 1)

static QueueHandle_t sensor_queue;

EventGroupHandle_t microSD_event_group;

void producer_task(void *params) {

  adc__initialize();
  LPC_ADC->CR &= ~((1 << 24) | (1 << 25) | (1 << 26));
  LPC_ADC->CR |= (1 << 5);
  adc__enable_burst_mode();
  pin_configure_adc_channel_as_io_pin(31); // p1.31 is sensor pin

  while (1) { // Assume 100ms loop - vTaskDelay(100)
    // fprintf(stderr, "PRODUCER\n");
    // Sample code:
    // 1. get_sensor_value()
    uint32_t sum_of_sensor_values = 0;
    for (int i = 0; i < 100; i++) {
      // uint16_t adc_value = 5;
      const uint16_t adc_value = adc__get_adc_value(ADC__CHANNEL_5);
      sum_of_sensor_values = sum_of_sensor_values + adc_value;
    }
    uint32_t average_sensor_value = sum_of_sensor_values / 100;
    xQueueSend(sensor_queue, &average_sensor_value, 0);
    xEventGroupSetBits(microSD_event_group, PRODUCER_BIT);
    // 2. xQueueSend(&handle, &sensor_value, 0);
    // 3. xEventGroupSetBits(checkin)
    // 4. vTaskDelay(100)
    vTaskDelay(100);
  }
}

void consumer_task(void *params) {
  const char *filename = "sensor.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_WRITE | FA_CREATE_ALWAYS));

  uint32_t sensor_value = 0;
  while (1) { // Assume 100ms loop
    // fprintf(stderr, "CONSUMER\n");
    xQueueReceive(sensor_queue, &sensor_value, portMAX_DELAY);
    const TickType_t time = xTaskGetTickCount();
    if (FR_OK == result) {
      char data[64];
      // sprintf(string, "Value,%i\n", 123);
      sprintf(data, "%i, %i\n", time, sensor_value);
      if (FR_OK == f_write(&file, data, strlen(data), &bytes_written)) {
      } else {
        printf("ERROR: Failed to write data to file\n");
      }
      f_sync(&file);
    } else {
      printf("ERROR: Failed to open: %s\n", filename);
    }

    fprintf(stderr, "%d\n", sensor_value);
    xEventGroupSetBits(microSD_event_group, CONSUMER_BIT);
  }
}

void watchdog_task(void *params) {
  EventBits_t check_in;
  const TickType_t xTicksToWait = 133 / portTICK_PERIOD_MS;

  while (1) {
    vTaskDelay(1000);
    /* Wait a maximum of 100ms for either bit 0 or bit 4 to be set within
    the event group.  Clear the bits before exiting. */
    check_in = xEventGroupWaitBits(microSD_event_group,         /* The event group being tested. */
                                   PRODUCER_BIT | CONSUMER_BIT, /* The bits within the event group to wait for. */
                                   pdTRUE,                      /* BIT_0 & BIT_4 should be cleared before returning. */
                                   pdTRUE,                      /* Don't wait for both bits, either bit will do. */
                                   xTicksToWait);               /* Wait a maximum of 100ms for either bit to be set. */
                                                                // ...
                                                                // vTaskDelay(200);
    // We either should vTaskDelay, but for better robustness, we should
    // block on xEventGroupWaitBits() for slightly more than 100ms because
    // of the expected production rate of the producer() task and its check-in

    if ((check_in & (PRODUCER_BIT | CONSUMER_BIT)) == (PRODUCER_BIT | CONSUMER_BIT)) {
      fprintf(stderr, "Both producer and cosumer checked in\n");
    } else if ((check_in & PRODUCER_BIT) != 0) {
      fprintf(stderr, "Only producer checked in\n");
    } else if ((check_in & CONSUMER_BIT) != 0) {
      fprintf(stderr, "Only consumer checked in\n");
    } else {
      fprintf(stderr, "Neither task checked in\n");
    }
  }
}

// void write_file_using_fatfs_pi(void) {
//   const char *filename = "file.txt";
//   FIL file; // File handle
//   UINT bytes_written = 0;
//   FRESULT result = f_open(&file, filename, (FA_WRITE | FA_CREATE_ALWAYS));

//   if (FR_OK == result) {
//     char string[64];
//     sprintf(string, "Value,%i\n", 123);
//     if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
//     } else {
//       printf("ERROR: Failed to write data to file\n");
//     }
//     f_close(&file);
//   } else {
//     printf("ERROR: Failed to open: %s\n", filename);
//   }
// }

void main(void) {

  xTaskCreate(producer_task, "producer", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(consumer_task, "consumer", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(watchdog_task, "watchdog", 1024 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  microSD_event_group = xEventGroupCreate();
  sensor_queue = xQueueCreate(1, sizeof(uint32_t));

  vTaskStartScheduler();
}

#endif

#if 0

static QueueHandle_t switch_queue;
static port_pin_s switch0 = {.port = 1, .pin = 19};

typedef enum { switch__off, switch__on } switch_e;

const switch_e get_switch_input_from_switch0(void) {
  switch_e switch_value;
  if (gpio_get_level(switch0)) {
    switch_value = switch__on;
  } else {
    switch_value = switch__off;
  }
  return (const)switch_value;
}

// TODO: Create this task at PRIORITY_LOW
void producer(void *p) {
  while (1) {
    // This xQueueSend() will internally switch context to "consumer" task because it is higher priority than this
    // "producer" task Then, when the consumer task sleeps, we will resume out of xQueueSend()and go over to the next
    // line

    // TODO: Get some input value from your board
    const switch_e switch_value = get_switch_input_from_switch0();

    // TODO: Print a message before xQueueSend()
    // Note: Use printf() and not fprintf(stderr, ...) because stderr is a polling printf
    fprintf(stderr, "%s(), line %d, sending\n", __FUNCTION__, __LINE__);
    xQueueSend(switch_queue, &switch_value, 0);
    fprintf(stderr, "%s(), line %d, done sending\n", __FUNCTION__, __LINE__);
    // TODO: Print a message after xQueueSend()

    vTaskDelay(1000);
  }
}

// TODO: Create this task at PRIORITY_HIGH
void consumer(void *p) {
  switch_e switch_value;
  while (1) {
    // TODO: Print a message before xQueueReceive()
    fprintf(stderr, "%s(), line %d, receiving\n", __FUNCTION__, __LINE__);
    xQueueReceive(switch_queue, &switch_value, portMAX_DELAY);
    fprintf(stderr, "%s(), line %d, done receiving\n", __FUNCTION__, __LINE__);

    // TODO: Print a message after xQueueReceive()
  }
}

// app_cli_status_e cli__task_control(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
//                                    app_cli__print_string_function cli_output) {
//   sl_string_t s = user_input_minus_command_name;

//   // If the user types 'taskcontrol suspend led0' then we need to suspend a task with the name of 'led0'
//   // In this case, the user_input_minus_command_name will be set to 'suspend led0' with the command-name removed
//   if (sl_string__begins_with_ignore_case(s, "suspend")) {
//     // TODO: Use sl_string API to remove the first word, such that variable 's' will equal to 'led0'
//     // TODO: Or you can do this: char name[16]; sl_string__scanf("%*s %16s", name);
//     char name[16];
//     sl_string__scanf("%*s %16s", name);
//     // Now try to query the tasks with the name 'led0'
//     TaskHandle_t task_handle = xTaskGetHandle(s);
//     if (NULL == task_handle) {
//       // note: we cannot use 'sl_string__printf("Failed to find %s", s);' because that would print existing string
//       onto
//       // itself
//       sl_string__insert_at(s, "Could not find a task with name:", NULL);
//       cli_output(NULL, s);
//     } else {
//       // TODO: Use vTaskSuspend()
//       vTaskSuspend(task_handle);
//       sl_string__insert_at(s, "Suspended: ", NULL);
//       cli_output(NULL, s);
//     }

//   } else if (sl_string__begins_with_ignore_case(s, "resume")) {
//     // TODO
//     char name[16];
//     sl_string__scanf("%*s %16s", name);
//     // Now try to query the tasks with the name 'led0'
//     TaskHandle_t task_handle = xTaskGetHandle(s);
//     if (NULL == task_handle) {
//       // note: we cannot use 'sl_string__printf("Failed to find %s", s);' because that would print existing string
//       onto
//       // itself
//       sl_string__insert_at(s, "Could not find a task with name:", NULL);
//       cli_output(NULL, s);
//     } else {
//       vTaskResume(task_handle);
//       sl_string__insert_at(s, "Resumed: ", NULL);
//       cli_output(NULL, s);
//     }

//   } else {
//     cli_output(NULL, "Did you mean to say suspend or resume?\n");
//   }

//   return APP_CLI_STATUS__SUCCESS;
// }

void main(void) {
  // onst uint32_t stack_task_size = 5000;
  gpio_set_as_input(switch0);

  xTaskCreate(producer, "producer", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(consumer, "consumer", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  // TODO Queue handle is not valid until you create it
  switch_queue =
      xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)

  vTaskStartScheduler();
}

#endif

// UART lab
#if 0

void uart_write_task(void *p);
void uart_read_task(void *p);
void create_uart_tasks(void);
void uart_intr_read(void *p);

int main(void) {
  init_uart(UART_2, 96, 38400);
  uart__enable_receive_interrupt(UART_2);
  create_uart_tasks();
  vTaskStartScheduler();
}

void uart_write_task(void *p) {
  while (1) {

    char to_be_sent = 'a';
    uart_polled_send(UART_2, to_be_sent);
    vTaskDelay(100);
  }
}

void uart_read_task(void *p) {
  while (1) {
    char receive = ' ';
    uart_polled_recieve(UART_2, &receive);
    // fprintf(stderr, "yo\n");
    fprintf(stderr, &receive);
    vTaskDelay(500);
  }
}

void uart_intr_read(void *p) {
  vTaskDelay(5000);
  while (1) {
    char byte_to_get = ' ';
    uart_lab__get_char_from_queue(&byte_to_get, portMAX_DELAY);
    fprintf(stderr, "%c\n", byte_to_get);
  }
}

void create_uart_tasks(void) {
  xTaskCreate(uart_write_task, "uart_write", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  // xTaskCreate(uart_read_task, "uart_read", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(uart_intr_read, "uart_intr_read", (512U * 4) / sizeof(void *), NULL, 1, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

// SPI lab
#if 0

static SemaphoreHandle_t spi_bus_mutex;
// TODO: Implement Adesto flash memory CS signal as a GPIO driver
// void adesto_cs(void);
// void adesto_ds(void);
void create_spi_tasks(void);

// TODO: Study the Adesto flash 'Manufacturer and Device ID' section
typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;

void print_adesto_info(adesto_flash_id_s data);
// TODO: Implement the code to read Adesto flash memory signature
// TODO: Create struct of type 'adesto_flash_id_s' and return it
adesto_flash_id_s adesto_read_signature(void) {
  adesto_flash_id_s data = {0};
  uint8_t dummy_byte = 0xFF;
  // adesto_cs();
  select_flash();
  (void)spi2_exchange_byte(0x9F);
  data.manufacturer_id = spi2_exchange_byte(dummy_byte);
  data.device_id_1 = spi2_exchange_byte(dummy_byte);
  data.device_id_2 = spi2_exchange_byte(dummy_byte);
  data.extended_device_id = spi2_exchange_byte(dummy_byte);
  // Send opcode and read bytes
  // TODO: Populate members of the 'adesto_flash_id_s' struct

  unselect_flash();
  // adesto_ds();

  return data;
}

void spi_task(void *p) {

  while (1) {
    if (xSemaphoreTake(spi_bus_mutex, 1000)) {
      const adesto_flash_id_s id = adesto_read_signature();

      // When we read a manufacturer ID we do not expect, we will kill this task
      if (0x1F != id.manufacturer_id) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      } else {
        print_adesto_info(id);
        // printf("0x%02X\n", id.manufacturer_id);
      }
      xSemaphoreGive(spi_bus_mutex);
    }
  }
}

int main(void) {
  spi_bus_mutex = xSemaphoreCreateMutex();
  init_spi();
  create_spi_tasks();
  vTaskStartScheduler();
}

void create_spi_tasks(void) {
  xTaskCreate(spi_task, "spi1", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(spi_task, "spi2", (512U * 4) / sizeof(void *), NULL, 1, NULL);
}

void print_adesto_info(adesto_flash_id_s data) {
  printf("0x%02X 0x%02X 0x%02X 0x%02X\n", data.manufacturer_id, data.device_id_1, data.device_id_2,
         data.extended_device_id);
}

#endif

// all labs before spi
#if 0
// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
static void led_task(void *pvParameters);
static void create_led_task(void);
static void switch_task(void *task_parameter);
static void create_switch_task(void);

static void led_task2(void *pvParameters);
static void create_ledtask_2(void);

void gpio_interrupt(void);
void configure_gpio_interrupt(void);
static void create_sleep_task(void);
static void sleep_task(void *task_parameter);
void clear_gpio_interrupt(void);

static SemaphoreHandle_t switch_press_indication;
static port_pin_s int_switch = {.port = 0, .pin = 29};
static port_pin_s int_led = {.port = 1, .pin = 26};
static port_pin_s int_led2 = {.port = 2, .pin = 3};
static port_pin_s status_led = {.port = 1, .pin = 24};

static SemaphoreHandle_t switch_pressed_signal;

static SemaphoreHandle_t switch_pressed_signal_30;
static SemaphoreHandle_t switch_pressed_signal_29;

void pin30_isr(void);
void pin29_isr(void);

void pwm_task(void *p);
static void create_pwm_task(void);
void pin_configure_pwm_channel_as_io_pin(void);

void adc_task(void *p);
static void create_adc_task(void);

static QueueHandle_t adc_to_pwm_task_queue;

int main(void) {
  adc_to_pwm_task_queue = xQueueCreate(1, sizeof(int));
  create_uart_task();
  create_adc_task();
  create_pwm_task();

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
}

static void create_adc_task(void) { xTaskCreate(adc_task, "adc", (512U * 4) / sizeof(void *), NULL, 1, NULL); }

void adc_task(void *p) {
  fprintf(stderr, "ENTERED ADC TASK");
  adc__initialize();

  // TODO This is the function you need to add to adc.h
  // You can configure burst mode for just the channel you are using
  LPC_ADC->CR &= ~((1 << 24) | (1 << 25) | (1 << 26));
  LPC_ADC->CR |= (1 << 5);
  adc__enable_burst_mode();

  // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
  // You can use gpio__construct_with_function() API from gpio.h
  pin_configure_adc_channel_as_io_pin(31); // TODO You need to write this function

  while (1) {

    // Get the ADC reading using a new routine you created to read an ADC burst reading
    // TODO: You need to write the implementation of this function
    const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_5);

    xQueueSend(adc_to_pwm_task_queue, &adc_value, 100);
    // fprintf(stderr, "adc value: %d\n", adc_value);

    vTaskDelay(100);
  }
}

static void create_pwm_task(void) { xTaskCreate(pwm_task, "pwm", (512U * 4) / sizeof(void *), NULL, 1, NULL); }

void pwm_task(void *p) {

  uint16_t adc_reading = 0;
  uint16_t percent = 0;

  while (1) {

    pwm1__init_single_edge(100);
    pin_configure_pwm_channel_as_io_pin();
    // Locate a GPIO pin that a PWM channel will control
    // NOTE You can use gpio__construct_with_function() API from gpio.h
    // TODO Write this function yourself

    // We only need to set PWM configuration once, and the HW will drive
    // the GPIO at 1000Hz, and control set its duty cycle to 50%
    // pwm1__set_duty_cycle(PWM1__2_0, 50);
    // fprintf(stderr, "yo");
    // Continue to vary the duty cycle in the loop
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 100)) {
      percent = (adc_reading * 100) / 4095;
      pwm1__set_duty_cycle(PWM1__2_0, percent);

      fprintf(stderr, "% value: %d\n", percent);
    }
  }
  // while (1) {
  //   pwm1__set_duty_cycle(PWM1__2_0, percent);

  //   if (++percent > 100) {
  //     percent = 0;
  //   }

  //   // vTaskDelay(100);
  // }
}

void pin_configure_pwm_channel_as_io_pin(void) {
  gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCTION_1); // set pin 0, port 2 -> PWM functionality
}

#if 0
int main(void) {
  // switch_press_indication = xSemaphoreCreateBinary();
  // create_blinky_tasks();
  switch_pressed_signal = xSemaphoreCreateBinary();
  switch_pressed_signal_30 = xSemaphoreCreateBinary();
  switch_pressed_signal_29 = xSemaphoreCreateBinary();
  create_uart_task();
  create_sleep_task();
  // create_led_task();
  // create_switch_task();

  // part0
  // LPC_GPIOINT->IO0IntEnF |= (1 << int_switch.pin);
  // NVIC_EnableIRQ(GPIO_IRQn); // ??
  // lpc_peripheral__enable_interrupt(GPIO_IRQn, gpio_interrupt, "int");

  // switch is falling edge

  // configure_gpio_interrupt();
  NVIC_EnableIRQ(GPIO_IRQn); // ??
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "unused");

  gpio0__attach_interrupt(30, GPIO_INTR__RISING_EDGE, pin30_isr);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, pin29_isr);

  LPC_IOCON->P2_3 &= ~(7 << 0);

  // gpio_set_as_input(int_switch);
  // gpio_set_as_output(status_led);
  // gpio_set_as_output(int_led);
  // LPC_GPIO1->DIR |= (1 << 24); // set LED2 as output

  // fprintf(stderr, "made it pass while loop");
  // If you have the ESP32 wifi module soldered on the board, you can try uncommenting this code
  // See esp32/README.md for more details
  // uart3_init();                                                                     // Also include:  uart3_init.h
  // xTaskCreate(esp32_tcp_hello_world_task, "uart3", 1000, NULL, PRIORITY_LOW, NULL); // Include esp32_task.h

  // xTaskCreate(led_task, "led", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
  while (1) {
    gpio_set_high(status_led);
    delay__ms(100);

    gpio_set_low(status_led);
    delay__ms(100);
  }
  return 0;
}
#endif

void pin30_isr(void) {
  xSemaphoreGiveFromISR(switch_pressed_signal_30, NULL);
  fprintf(stderr, " pin 30 interrupt ");
}

void pin29_isr(void) {
  xSemaphoreGiveFromISR(switch_pressed_signal_29, NULL);
  fprintf(stderr, " pin 29 interrupt ");
}

static void sleep_task(void *task_parameter) {
  while (true) {
    if (xSemaphoreTakeFromISR(switch_pressed_signal, portMAX_DELAY)) {
      for (int i = 0; i < 15; i++) {
        gpio_set_high(int_led);
        vTaskDelay(50);
        gpio_set_low(int_led);
        vTaskDelay(50);
      }
    }
    if (xSemaphoreTakeFromISR(switch_pressed_signal_30, portMAX_DELAY)) {
      for (int i = 0; i < 20; i++) {
        gpio_set_high(int_led);
        vTaskDelay(25);
        gpio_set_low(int_led);
        vTaskDelay(25);
      }
    }
    if (xSemaphoreTakeFromISR(switch_pressed_signal_29, portMAX_DELAY)) {
      for (int i = 0; i < 6; i++) {
        gpio_set_high(int_led2);
        vTaskDelay(100);
        gpio_set_low(int_led2);
        vTaskDelay(100);
      }
    }
  }
}

static void create_sleep_task(void) {
  fprintf(stderr, "create sleep task");
  xTaskCreate(sleep_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
}

void clear_gpio_interrupt(void) { LPC_GPIOINT->IO0IntClr |= (1 << int_switch.pin); }

void configure_gpio_interrupt(void) { LPC_GPIOINT->IO0IntEnF |= (1 << int_switch.pin); }
// Step 2:
void gpio_interrupt(void) {
  fprintf(stderr, "enter interrupt");
  // delay__ms(30); // account for switch debouncing
  // a) Clear Port0/2 interrupt using CLR0 or CLR2 registers
  // b) Use fprintf(stderr) or blink and LED here to test your ISR
  // part 0
  // LPC_GPIOINT->IO0IntClr |= (1 << int_switch.pin); // clear reg at switch

  // if (gpio_get_level(int_led)) {
  //   gpio_set_low(int_led);
  // } else {
  //   gpio_set_high(int_led);
  // }
  clear_gpio_interrupt();
  // xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
}

/*
static void led_task(void *pvParameters) {
  // Choose one of the onboard LEDS by looking into schematics and write code for the below
  // 0) Set the IOCON MUX function(if required) select pins to 000
  LPC_IOCON->P2_3 &= ~(7 << 0);

  LPC_GPIO1->DIR |= (1 << 24);

  while (true) {
    LPC_GPIO1->SET = (1 << 24);
    vTaskDelay(500);

    LPC_GPIO1->CLR = (1 << 24);
    vTaskDelay(500);
  }
}
*/

static void led_task(void *task_parameter) {
  LPC_IOCON->P2_3 &= ~(7 << 0);
  // Type-cast the paramter that was passed from xTaskCreate()
  const port_pin_s *led = (port_pin_s *)(task_parameter);

  while (true) {
    // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore (signal)
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      // TODO: Blink the LED

      // very crude PWM
      for (int i = 0; i < 25; i++) {
        gpio_set_high(*led);
        vTaskDelay(i);
        gpio_set_low(*led);
        vTaskDelay(25 - i);
      }
      for (int i = 25; i > 0; i--) {
        gpio_set_high(*led);
        vTaskDelay(i);
        gpio_set_low(*led);
        vTaskDelay(25 - i);
      }
    } else {
      puts("Timeout: No switch press indication for 1000ms");
    }
  }
}

static void switch_task(void *task_parameter) {
  port_pin_s *sw = (port_pin_s *)task_parameter;

  while (true) {
    // TODO: If switch pressed, set the binary semaphore
    if (gpio_get_level(*sw)) {
      xSemaphoreGive(switch_press_indication);
    }

    // Task should always sleep otherwise they will use 100% CPU
    // This task sleep also helps avoid spurious semaphore give during switch debeounce
    vTaskDelay(100);
  }
}

// static void led_task2(void *task_parameter) {
//   LPC_IOCON->P2_3 &= ~(7 << 0);
//   // Type-cast the paramter that was passed from xTaskCreate()
//   const port_pin_s *led = (port_pin_s *)(task_parameter);

//   while (true) {
//     if (xSemaphoreTake(toggle_led, 1000)) {
//       gpio_set_high(*led);
//       vTaskDelay(100);

//       gpio_set_low(*led);
//       vTaskDelay(100);
//     }
//   }
// }

// static void create_ledtask_2(void) {
//   static port_pin_s led1 = {.port = 1, .pin = 24};
//   xTaskCreate(led_task2, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
// }

static void create_switch_task(void) {
  static port_pin_s sw3 = {.port = 0, .pin = 29};

  xTaskCreate(switch_task, "sw3", configMINIMAL_STACK_SIZE, (void *)&sw3, PRIORITY_LOW, NULL);
}

static void create_led_task(void) {
  static port_pin_s led0 = {.port = 2, .pin = 3};
  static port_pin_s led1 = {.port = 1, .pin = 26};
  static port_pin_s led2 = {.port = 1, .pin = 24};
  static port_pin_s led3 = {.port = 1, .pin = 18};

  xTaskCreate(led_task, "led0", configMINIMAL_STACK_SIZE, (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(led_task, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
  xTaskCreate(led_task, "led2", configMINIMAL_STACK_SIZE, (void *)&led2, PRIORITY_LOW, NULL);
  xTaskCreate(led_task, "led3", configMINIMAL_STACK_SIZE, (void *)&led3, PRIORITY_LOW, NULL);
}

static void create_blinky_tasks(void) {
  /**
   * Use '#if (1)' if you wish to observe how two tasks can blink LEDs
   * Use '#if (0)' if you wish to use the 'periodic_scheduler.h' that will spawn 4 periodic tasks, one for each LED
   */
#if (1)
  // These variables should not go out of scope because the 'blink_task' will reference this memory
  static gpio_s led0, led1;

  led0 = board_io__get_led0();
  led1 = board_io__get_led1();

  xTaskCreate(blink_task, "led0", configMINIMAL_STACK_SIZE, (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
#else
  const bool run_1000hz = true;
  const size_t stack_size_bytes = 2048 / sizeof(void *); // RTOS stack size is in terms of 32-bits for ARM M4 32-bit CPU
  periodic_scheduler__initialize(stack_size_bytes, !run_1000hz); // Assuming we do not need the high rate 1000Hz task
  UNUSED(blink_task);
#endif
}

static void create_uart_task(void) {
  // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
  // Change '#if (0)' to '#if (1)' and vice versa to try it out
#if (0)
  // printf() takes more stack space, size this tasks' stack higher
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#else
  sj2_cli__init();
  UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
#endif
}

static void blink_task(void *params) {
  const gpio_s led = *((gpio_s *)params); // Parameter was input while calling xTaskCreate()

  // Warning: This task starts with very minimal stack, so do not use printf() API here to avoid stack overflow
  while (true) {
    gpio__toggle(led);
    vTaskDelay(500);
  }
}

// This sends periodic messages over printf() which uses system_calls.c to send them to UART0
static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
     *  - During debugging in case system crashes before all output of printf() is sent
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}
#endif