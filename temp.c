// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #include "FreeRTOS.h"
// #include "semphr.h"
// #include "task.h"

// #include "acceleration.h"
// #include "adc.h"
// #include "board_io.h"
// #include "common_macros.h"
// #include "event_groups.h"
// #include "ff.h"
// #include "gpio_isr.h"
// #include "gpio_lab.h"
// #include "i2c.h"
// #include "lpc40xx.h"
// #include "lpc_peripherals.h"
// #include "periodic_scheduler.h"
// #include "pwm1.h"
// #include "queue.h"
// #include "sj2_cli.h"
// #include "spi_mp3.h"
// #include "uart_lab.h"
// #include <time.h>

// static void create_uart_task(void);
// static void uart_task(void *params);

// QueueHandle_t Q_songname;
// QueueHandle_t Q_songdata;

// static void mp3_reader_task(void *p);
// static void mp3_player_task(void *p);
// static bool mp3_decoder_needs_data(void);
// static bool play_next_track(void);
// static bool play_previous_track(void);
// static bool pause(void);
// static void print_transfer(double file_size, uint32_t kilobytes);
// static void print_bytes(char song_data[]);
// #define DATA_TRANSFER_SIZE 1024
// #define SONG_NAME_SIZE 32
// char song_name[SONG_NAME_SIZE];

// // uint8_t slave_address_to_respond_to = 0x10;
// // i2c2__slave_init(slave_address_to_respond_to);
// // i2c__detect(I2C__2, slave_address_to_respond_to);
// /*******************************************************************************
//  *
//  *                                    Main
//  *
//  ******************************************************************************/
// int main(void) {
//   create_uart_task();
//   puts("Starting RTOS");

//   const uint32_t kilobyte = 1024;
//   uint32_t max_clock_mhz = 1; // CLKI/7. 12.288/7 = 1.755Mhz Max Clock

//   Q_songname = xQueueCreate(1, sizeof(song_name));
//   Q_songdata = xQueueCreate(2, DATA_TRANSFER_SIZE);

//   spi0__init(max_clock_mhz);

//   xTaskCreate(mp3_reader_task, "reader", (4 * kilobyte) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
//   xTaskCreate(mp3_player_task, "player", (4 * kilobyte) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

//   vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
//   return 0;
// }

// /*******************************************************************************
//  *
//  *                              Other Functions
//  *
//  ******************************************************************************/

// static void create_uart_task(void) {
//   // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
//   // Change '#if (0)' to '#if (1)' and vice versa to try it out
// #if (0)
//   // printf() takes more stack space, size this tasks' stack higher
//   xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
// #else
//   sj2_cli__init();
//   UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
// #endif
// }

// // This sends periodic messages over printf() which uses system_calls.c to send them to UART0
// static void uart_task(void *params) {
//   TickType_t previous_tick = 0;
//   TickType_t ticks = 0;

//   while (true) {
//     // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
//     vTaskDelayUntil(&previous_tick, 2000);

//     /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
//      * sent out before this function returns. See system_calls.c for actual implementation.
//      *
//      * Use this style print for:
//      *  - Interrupts because you cannot use printf() inside an ISR
//      *    This is because regular printf() leads down to xQueueSend() that might block
//      *    but you cannot block inside an ISR hence the system might crash
//      *  - During debugging in case system crashes before all output of printf() is sent
//      */
//     ticks = xTaskGetTickCount();
//     fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
//     fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

//     /* This deposits data to an outgoing queue and doesn't block the CPU
//      * Data will be sent later, but this function would return earlier
//      */
//     ticks = xTaskGetTickCount();
//     printf("This is a more efficient printf ... finished in");
//     printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
//   }
// }

// /*******************************************************************************
//  *
//  *                                MP3 PROJECT
//  *
//  ******************************************************************************/

// static void mp3_reader_task(void *p) {
//   char song_data[DATA_TRANSFER_SIZE];
//   FIL file;    /* File objects */
//   UINT br = 0; /* File read/write count */

//   while (true) {
//     xQueueReceive(Q_songname, &song_name[0], portMAX_DELAY);
//     FRESULT result = f_open(&file, song_name, FA_READ);
//     if (FR_OK == result) {
//       fprintf(stdout, "File opened: %s\n", song_name);
//       while (!f_eof(&file)) {
//         f_read(&file, song_data, sizeof(song_data), &br);
//         if (br == 0) {
//           break; // error or eof
//         }
//         xQueueSend(Q_songdata, &song_data[0], portMAX_DELAY);
//       }
//       f_close(&file);
//       printf("File closed: %s\n", song_name);
//     } else {
//       fprintf(stderr, "ERROR: Failed to open file: %s\n", song_name);
//     }
//   }
// }

// static void mp3_player_task(void *p) {
//   char song_data[DATA_TRANSFER_SIZE];
//   // uint32_t kilobyte = 0;
//   // double file_size = 10211114;

//   while (true) {
//     xQueueReceive(Q_songdata, &song_data[0], portMAX_DELAY);
//     for (int i = 0; i < sizeof(song_data); i++) {
//       while (!mp3_decoder_needs_data()) {
//         ;
//       }
//       spi_send_data_to_mp3_decoder(song_data[i]);
//     }
//   }
// }

// static bool play_next_track(void) { return false; }

// static bool play_previous_track(void) { return false; }

// static bool pause(void) { return false; }

// static void print_transfer(double file_size, uint32_t kilobytes) {
//   uint32_t bytes = 1024 * kilobytes;
//   uint32_t megabytes = kilobytes / 1000;
//   double percent = (bytes / file_size) * 100;
//   fprintf(stderr, "Transfer %.2f%% %lumb %lukb %lub\n", percent, megabytes, kilobytes, bytes);
// }

// static void print_bytes(char song_data[]) {
//   for (int byte = 0; byte < DATA_TRANSFER_SIZE; byte++) {
//     fprintf(stderr, "%x", song_data[byte]);
//   }
//   fprintf(stderr, "\n");
// }

// #pragma once
// #include <stdint.h>

// void spi0__init(uint32_t max_clock_mhz);

// void spi_send_data_to_mp3_decoder(uint8_t song_data);

// void mp3_set_volume(uint16_t volume);

// void mp3_write_register(uint8_t address_byte, uint16_t data);

// uint16_t mp3_read_register(uint8_t address_byte);

// #include "LPC40xx.h"
// #include "clock.h"
// #include "gpio.h"
// #include "spi_mp3.h"
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>

// /*******************************************************************************
//  *
//  *               P R I V A T E    D A T A    D E F I N I T I O N S
//  *
//  ******************************************************************************/

// // VS10xx SCI Registers
// #define SCI_MODE 0x00
// #define SCI_STATUS 0x01
// #define SCI_BASS 0x02
// #define SCI_CLOCKF 0x03
// #define SCI_DECODE_TIME 0x04
// #define SCI_AUDATA 0x05
// #define SCI_WRAM 0x06
// #define SCI_WRAMADDR 0x07
// #define SCI_HDAT0 0x08
// #define SCI_HDAT1 0x09
// #define SCI_AIADDR 0x0A
// #define SCI_VOL 0x0B
// #define SCI_AICTRL0 0x0C
// #define SCI_AICTRL1 0x0D
// #define SCI_AICTRL2 0x0E
// #define SCI_AICTRL3 0x0F

// static const uint32_t mp3_dcs = (1 << 0);
// static const uint32_t mp3_dreq = (1 << 1);
// static const uint32_t mp3_rst = (1 << 16);
// static const uint32_t mp3_cs = (1 << 22);

// /*******************************************************************************
//  *
//  *        P R I V A T E    F U N C T I O N S    D E C L A R A T I O N S
//  *
//  ******************************************************************************/

// static void mp3_decoder_cs_activate(void);
// static void mp3_decoder_cs_deactivate(void);
// static void mp3_decoder_dcs_activate(void);
// static void mp3_decoder_dcs_deactivate(void);
// static void mp3_decoder_rst(void);
// static void spi0__init_pins(void);
// static void spi0__power_on_peripheral(void);
// static void spi0__init_ctrl_registers(void);
// static void spi0__init_clock_prescalar(uint32_t max_clock_mhz);
// static void spi0__init_peripherals(uint32_t max_clock_mhz);
// static void mp3_decoder__init(void);
// static uint8_t spi0__exchange_byte(uint8_t data_out);

// /*******************************************************************************
//  *
//  *                     P R I V A T E    F U N C T I O N S
//  *
//  ******************************************************************************/

// static void mp3_decoder_cs_activate(void) { LPC_GPIO0->CLR = mp3_cs; }
// static void mp3_decoder_cs_deactivate(void) { LPC_GPIO0->SET = mp3_cs; }
// static void mp3_decoder_dcs_activate(void) { LPC_GPIO0->CLR = mp3_dcs; }
// static void mp3_decoder_dcs_deactivate(void) { LPC_GPIO0->SET = mp3_dcs; }
// static void mp3_decoder_rst(void) {
//   LPC_GPIO0->CLR = mp3_rst;
//   LPC_GPIO0->SET = mp3_rst;
// }

// static void spi0__init_pins(void) {
//   // port & pin function reset
//   LPC_IOCON->P0_0 &= ~(0b111 << 0);  // dcs
//   LPC_IOCON->P0_1 &= ~(0b111 << 0);  // dreq
//   LPC_IOCON->P0_15 &= ~(0b111 << 0); // sck
//   LPC_IOCON->P0_16 &= ~(0b111 << 0); // rst
//   LPC_IOCON->P0_17 &= ~(0b111 << 0); // miso
//   LPC_IOCON->P0_18 &= ~(0b111 << 0); // mosi
//   LPC_IOCON->P0_22 &= ~(0b111 << 0); // cs

//   // port & pin function set
//   LPC_IOCON->P0_0 |= (0b000 << 0);  // dcs  gpio
//   LPC_IOCON->P0_1 |= (0b000 << 0);  // dreq gpio
//   LPC_IOCON->P0_15 |= (0b010 << 0); // sck
//   LPC_IOCON->P0_16 |= (0b000 << 0); // rst  gpio
//   LPC_IOCON->P0_17 |= (0b010 << 0); // miso
//   LPC_IOCON->P0_18 |= (0b010 << 0); // mosi
//   LPC_IOCON->P0_22 |= (0b000 << 0); // cs   gpio

//   // gpio port & pin direction
//   LPC_GPIO0->DIR |= mp3_dcs;   // dcs  output
//   LPC_GPIO0->DIR &= ~mp3_dreq; // dreq input
//   LPC_GPIO0->DIR |= mp3_rst;   // rst  output
//   LPC_GPIO0->DIR |= mp3_cs;    // cs   output

//   mp3_decoder_cs_deactivate();
//   mp3_decoder_dcs_deactivate();
// }

// static void spi0__power_on_peripheral(void) {
//   const uint32_t power_on_ssp0 = (1 << 21);
//   LPC_SC->PCONP |= power_on_ssp0;
// }

// static void spi0__init_ctrl_registers(void) {
//   uint8_t spi_enable = (0b1 << 1);
//   uint8_t bit8_data_transfer = (0b0111 << 0);
//   uint8_t spi_frame_formate = (0b00 << 4);
//   LPC_SSP0->CR0 |= bit8_data_transfer | spi_frame_formate;
//   LPC_SSP0->CR1 |= spi_enable;
// }

// static void spi0__init_clock_prescalar(uint32_t max_clock_mhz) {
//   uint8_t divider = 2;
//   const uint32_t cpu_clock_mhz = clock__get_core_clock_hz();
//   while (max_clock_mhz < (cpu_clock_mhz / divider) && divider <= 254) {
//     divider += 2;
//   }
//   LPC_SSP0->CPSR = divider;
// }

// static void spi0__init_peripherals(uint32_t max_clock_mhz) {
//   spi0__power_on_peripheral();
//   spi0__init_ctrl_registers();
//   spi0__init_clock_prescalar(max_clock_mhz);
// }

// static void spi0_busy_wait(void) {
//   const uint32_t spi_busy = (1 << 4);
//   while (LPC_SSP0->SR & spi_busy) {
//     ;
//   }
// }

// static void mp3_decoder__init(void) {
//   mp3_decoder_rst();
//   mp3_set_volume(0x2020);
//   uint16_t mp3_mode = mp3_read_register(SCI_MODE);
//   uint16_t mp3_status = mp3_read_register(SCI_STATUS);
//   uint16_t mp3_clock = mp3_read_register(SCI_CLOCKF);
//   uint16_t mp3_volume = mp3_read_register(SCI_VOL);
//   fprintf(stderr, "Mode:0x%04x Status:0x%04x Clock:0x%04x Vol:0x%04x\n", mp3_mode, mp3_status, mp3_clock,
//   mp3_volume); mp3_write_register(SCI_CLOCKF, 0x8800); // 12.288 * 3.5 = ~36 spi0__init_clock_prescalar(4);
// }

// static uint8_t spi0__exchange_byte(uint8_t data_out) {
//   LPC_SSP0->DR = data_out;
//   spi0_busy_wait();
//   return LPC_SSP0->DR;
// }

// /*******************************************************************************
//  *
//  *                      P U B L I C    F U N C T I O N S
//  *
//  ******************************************************************************/

// void spi0__init(uint32_t max_clock_mhz) {
//   spi0__init_pins();
//   spi0__init_peripherals(max_clock_mhz);
//   mp3_decoder__init();
// }

// bool mp3_decoder_needs_data(void) { return LPC_GPIO0->PIN & mp3_dreq; }

// void spi_send_data_to_mp3_decoder(uint8_t song_data) {
//   mp3_decoder_dcs_activate();
//   (void)spi0__exchange_byte(song_data);
//   mp3_decoder_dcs_deactivate();
// }

// void mp3_set_volume(uint16_t volume) { mp3_write_register(SCI_VOL, volume); }

// void mp3_write_register(uint8_t address_byte, uint16_t data) {
//   uint8_t low_byte = data;
//   uint8_t high_byte = data >> 8;
//   uint8_t write_op = 0x02;
//   while (!mp3_decoder_needs_data()) {
//     ;
//   }
//   mp3_decoder_cs_activate();
//   (void)spi0__exchange_byte(write_op);
//   (void)spi0__exchange_byte(address_byte);
//   (void)spi0__exchange_byte(high_byte);
//   (void)spi0__exchange_byte(low_byte);
//   mp3_decoder_cs_deactivate();
// }

// uint16_t mp3_read_register(uint8_t address_byte) {
//   uint8_t dummy_byte = 0xFF;
//   uint8_t read_op = 0x03;
//   while (!mp3_decoder_needs_data()) {
//     ;
//   }
//   mp3_decoder_cs_activate();
//   (void)spi0__exchange_byte(read_op);
//   (void)spi0__exchange_byte(address_byte);
//   uint8_t response_high_byte = spi0__exchange_byte(dummy_byte);
//   uint8_t response_low_byte = spi0__exchange_byte(dummy_byte);
//   mp3_decoder_cs_deactivate();
//   uint16_t complete_response = response_high_byte << 8;
//   return complete_response | response_low_byte;
// }
