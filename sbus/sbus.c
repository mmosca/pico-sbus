#include <stdio.h>
#include <string.h>

#include <assert.h>

#include "sbus.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/sync.h"

#include "uart_8e2_rx.pio.h"

volatile int irq_count = 0;

volatile uint8_t sbus_data[SBUS_FIFO_SIZE][SBUS_MESSAGE_MAX_SIZE];
volatile uint8_t current_sbus_data[SBUS_MESSAGE_MAX_SIZE];
volatile uint8_t oldest = 0;
volatile uint8_t newest = 0;
volatile uint8_t stored = 0;
volatile int sbus_index = 0;
volatile bool hasStartByte = false;


static uart_inst_t *sbus_uart_id;
uint sm;
PIO sbus_pio;

critical_section_t fifo_lock;

//#define DEBUG

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

#define IS_SET(val, bit) (val & (1 << bit))

typedef struct {
  uint8_t idx;
  uint8_t shift1;
  uint8_t shift2;
  uint8_t shift3; // 11 = byte 3 ignored
} chinfo_t;


static const chinfo_t CHINFO[16] = { {0,  0, 8, 11}, {1,  3, 5, 11}, {2,  6, 2, 10}, {4,  1, 7, 11}, {5,  4, 4, 11},
                                           {6,  7, 1, 9},  {8,  2, 6, 11}, {9,  5, 3, 11}, {11, 0, 8, 11}, {12, 3, 5, 11},
                                           {13, 6, 2, 10}, {15, 1, 7, 11}, {16, 4, 4, 11}, {17, 7, 1, 9},  {19, 2, 6, 11},  {20, 5, 3, 11} };


void decode_sbus_data(const uint8_t *data, sbus_state_t *decoded)
{
    for(int channel = 0; channel < 16; ++channel)
    {
        const chinfo_t *info = &CHINFO[channel];
        int idx = info->idx + 1;
        uint8_t b1 = data[idx];
        uint8_t b2 = data[idx+1];
        uint8_t b3 = data[idx+2];

        uint16_t chData = ((b1 >> info->shift1) | (b2 << info->shift2) | (b3 << info->shift3)) & 0x7FF;

        assert(chData > 2048);

        decoded->ch[channel] = chData;
    }

    decoded->ch[16] = data[23] & 1 ? 0x7FF : 0;
    decoded->ch[17] = data[23] & (1<<1) ? 0x7FF : 0;
    decoded->framelost = data[23] & (1<<2);
    decoded->failsafe = data[23] & (1<<3);
}

void sbus_pio_init(PIO pio, int rx_pin, int dbg_pin)
{
    // init mutex
    critical_section_init(&fifo_lock);
    // clear fifo
    for(int i = 0; i < SBUS_FIFO_SIZE; ++i)
    {
        memset((void *)sbus_data[i], 0, SBUS_MESSAGE_MAX_SIZE);
    }
    oldest = newest = stored = 0;

    sm = 0;
    uint offset = 0;
    uint baud = 100000;
    uart_8e2_rx_program_init(pio, sm, offset, rx_pin, baud, dbg_pin);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int PIO_IRQ = pio == pio0 ? PIO0_IRQ_0 : PIO1_IRQ_0;
    sbus_pio = pio;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(PIO_IRQ, sbus_on_pio_rx);
    irq_set_enabled(PIO_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    pio_set_irq0_source_enabled(pio, pis_sm0_rx_fifo_not_empty , true);
}

void sbus_init(uart_inst_t *uart, int rx_pin, int tx_pin)
{
    sbus_uart_id = uart;
    // init mutex
    critical_section_init(&fifo_lock);
    // clear fifo
    for(int i = 0; i < SBUS_FIFO_SIZE; ++i)
    {
        memset((void *)sbus_data[i], 0, SBUS_MESSAGE_MAX_SIZE);
    }
    oldest = newest = stored = 0;

    uart_init(uart, 115200);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);


    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int actual = uart_set_baudrate(uart, SBUS_BAUD_RATE);

    printf("Actual baud rate: %i\n", actual);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(uart, false, false);

    // Set our data format
    uart_set_format(uart, SBUS_DATA_BITS, SBUS_STOP_BITS, SBUS_PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(uart, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = uart == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, sbus_on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(uart, true, false);

}

bool hasSbusData()
{
    return stored > 0 && oldest != newest;
}

bool readSbusData(uint8_t *data)
{
    bool ret = false;
    critical_section_enter_blocking(&fifo_lock);
    if(hasSbusData())
    {
        memcpy((void *)data, (void *)sbus_data[oldest], SBUS_MESSAGE_MAX_SIZE);
        oldest = (oldest + 1) % SBUS_FIFO_SIZE;
        stored--;
        ret = true;
    }
    critical_section_exit(&fifo_lock);

    return ret;
}

// RX interrupt handler
// Do not print or wait
void sbus_on_uart_rx() {
    irq_count++;
    while (uart_is_readable(sbus_uart_id)) {
        uint8_t ch = uart_getc(sbus_uart_id);
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

            if(current_sbus_data[SBUS_MESSAGE_MAX_SIZE - 1] == SBUS_ENDBYTE ||
                (current_sbus_data[SBUS_MESSAGE_MAX_SIZE - 1] & SBUS2_ENDBYTE_MASK) == SBUS2_ENDBYTE)
            {
                critical_section_enter_blocking(&fifo_lock);
                uint8_t nextNewest = (newest + 1) % SBUS_FIFO_SIZE;
                // full package
                memcpy((void *)sbus_data[nextNewest], (void *)current_sbus_data, SBUS_MESSAGE_MAX_SIZE);
                newest = nextNewest;
                if(oldest = nextNewest)
                {
                    oldest = (oldest + 1) % SBUS_FIFO_SIZE;
                }

                stored++;
                if(stored > SBUS_FIFO_SIZE)
                {
                    stored = SBUS_FIFO_SIZE;
                }
                critical_section_exit(&fifo_lock);
            }
        }
    }
}

// RX interrupt handler
// Do not print or wait
void sbus_on_pio_rx() {
    irq_count++;


    while (pio_is_readable(sbus_pio, sm)) {
        uint8_t ch = pio_getc(sbus_pio, sm);
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

            if(current_sbus_data[SBUS_MESSAGE_MAX_SIZE - 1] == SBUS_ENDBYTE ||
                (current_sbus_data[SBUS_MESSAGE_MAX_SIZE - 1] & SBUS2_ENDBYTE_MASK) == SBUS2_ENDBYTE)
            {
                critical_section_enter_blocking(&fifo_lock);
                uint8_t nextNewest = (newest + 1) % SBUS_FIFO_SIZE;
                // full package
                memcpy((void *)sbus_data[nextNewest], (void *)current_sbus_data, SBUS_MESSAGE_MAX_SIZE);
                newest = nextNewest;
                if(oldest = nextNewest)
                {
                    oldest = (oldest + 1) % SBUS_FIFO_SIZE;
                }

                stored++;
                if(stored > SBUS_FIFO_SIZE)
                {
                    stored = SBUS_FIFO_SIZE;
                }
                critical_section_exit(&fifo_lock);
            }
        }
    }
}

