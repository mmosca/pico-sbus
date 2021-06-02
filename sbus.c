#include <stdio.h>
#include <string.h>

#include "sbus.h"


#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

volatile int irq_count = 0;

volatile uint8_t sbus_data[SBUS_FIFO_SIZE][SBUS_MESSAGE_MAX_SIZE];
volatile uint8_t current_sbus_data[SBUS_MESSAGE_MAX_SIZE];
volatile uint8_t oldest = 0;
volatile uint8_t newest = 0;
volatile uint8_t stored = 0;
volatile int sbus_index = 0;
volatile bool hasStartByte = false;

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifdef DEBUG
#define D(X) (X)
#else
#define D(X)
#endif


void decode_sbus_data(const uint8_t *data, sbus_state_t *decoded)
{
    for(int channel = 0; channel < 16; ++channel)
    {
        uint16_t chData = 0;
        int firstBit = channel * 11;

        int byte = (firstBit / 8) + 1;

        int bitsInFirstByte = 8 - (firstBit % 8);
        int bitsInSecondByte = MIN(11 - bitsInFirstByte, 8);
        int bitsInThirdByte = 11 - (bitsInFirstByte + bitsInSecondByte);
        D(printf("Ch%i: %i + %i + %i\n", channel + 1, bitsInFirstByte, bitsInSecondByte, bitsInThirdByte));

        for(int i = bitsInFirstByte - 1; i >= 0; i--)
        {
            D(printf("Byte1: bit %i (%i)\n", i, bitsInFirstByte));
            int bit = data[byte] & (1<<i);
            chData <<= 1;
            chData |= bit ? 1 : 0;
        }

        for(int i = 0; i < bitsInSecondByte; i++)
        {
            int shift = 7 - i;
            D(printf("Byte2: bit %i (%i)\n", shift, bitsInSecondByte));
            int bit = data[byte + 1] & (1 << shift);
            chData <<= 1;
            chData |= bit ? 1 : 0;
        }

        for(int i = 0; i < bitsInThirdByte; i++)
        {
            int shift = 7 - i;
            D(printf("Byte3: bit %i (%i)\n", shift, bitsInThirdByte));
            int bit = data[byte + 2] & (1 << shift);
            chData <<= 1;
            chData |= bit ? 1 : 0;
        }

        if(channel == 0) {
            decoded->ch1 = chData;
        } else if(channel == 1) {
            decoded->ch2 = chData;
        } else if(channel == 2) {
            decoded->ch3 = chData;
        } else if(channel == 3) {
            decoded->ch4 = chData;
        } else if(channel == 4) {
            decoded->ch5 = chData;
        } else if(channel == 5) {
            decoded->ch6 = chData;
        } else if(channel == 6) {
            decoded->ch7 = chData;
        } else if(channel == 7) {
            decoded->ch8 = chData;
        } else if(channel == 8) {
            decoded->ch9 = chData;
        } else if(channel == 9) {
            decoded->ch10 = chData;
        } else if(channel == 10) {
            decoded->ch11 = chData;
        } else if(channel == 11) {
            decoded->ch12 = chData;
        } else if(channel == 12) {
            decoded->ch13 = chData;
        } else if(channel == 13) {
            decoded->ch14 = chData;
        } else if(channel == 14) {
            decoded->ch15 = chData;
        } else if(channel == 15) {
            decoded->ch16 = chData;
        }
    }

    decoded->dch17 = data[23] & 0x80;
    decoded->dch18 = data[23] & 0x40;
    decoded->framelost = data[23] & 0x20;
    decoded->failsafe = data[23] & 0x10;
}

bool hasSbusData()
{
    return stored > 0;
}

void readSbusData(uint8_t *data)
{
    if(hasSbusData())
    {
        memcpy((void *)data, (void *)sbus_data[oldest], SBUS_MESSAGE_MAX_SIZE);
        oldest = (oldest + 1) % SBUS_FIFO_SIZE;
        stored--;
    }
}

// RX interrupt handler
// Do not print or wait
void sbus_on_uart_rx() {
    irq_count++;
    while (uart_is_readable(SBUS_UART_ID)) {
        uint8_t ch = uart_getc(SBUS_UART_ID);
        if(!hasStartByte && ch != SBUS_STARTBYTE)
        {
            continue;
        }

        hasStartByte = true;
        current_sbus_data[sbus_index++] = ch;

        if(sbus_index == SBUS_MESSAGE_MAX_SIZE)
        {
            hasStartByte = false;
            sbus_index = 0;

            if(current_sbus_data[SBUS_MESSAGE_MAX_SIZE - 1] == SBUS_ENDBYTE)
            {
                uint8_t nextNewest = (newest + 1) % SBUS_FIFO_SIZE;
                // full package
                memcpy((void *)sbus_data[nextNewest], (void *)current_sbus_data, SBUS_MESSAGE_MAX_SIZE);
                newest = nextNewest;
                stored++;
                if(stored > SBUS_FIFO_SIZE)
                {
                    oldest =  (oldest + 1) % SBUS_FIFO_SIZE;
                    stored = SBUS_FIFO_SIZE;
                }
            }
        }
    }

    //gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

