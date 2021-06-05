#ifndef __SBUS_H__

#include <stdint.h>
#include <stdbool.h>

#include "hardware/uart.h"

#define SBUS_UART_ID uart1
// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define SBUS_BAUD_RATE 100000
#define SBUS_DATA_BITS 8
#define SBUS_PARITY    UART_PARITY_EVEN
#define SBUS_STOP_BITS 2

#define SBUS_STARTBYTE	0x0F
#define SBUS_ENDBYTE	0X00
#define SBUS_MESSAGE_MAX_SIZE 25
#define SBUS_CHANNEL_BIT_MASK   0x7FF
#define SBUS_CHANNEL_BIT_MASK_NO_FIRST_BIT (SBUS_CHANNEL_BITS & ~1)

#define SBUS_FIFO_SIZE	2

typedef struct {
    uint16_t ch[18];
    bool framelost;
    bool failsafe;
} sbus_state_t;

bool hasSbusData();

bool readSbusData(uint8_t *data);

void sbus_on_uart_rx();

void decode_sbus_data(const uint8_t *data, sbus_state_t *decoded);

void sbus_init(uart_inst_t *uart, int rx_pin, int tx_pin);

#endif
