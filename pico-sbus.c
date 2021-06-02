#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"


#define UART_ID uart0
#define SBUS_BAUD_RATE 100000
#define SBUS_DATA_BITS 8
#define SBUS_PARITY    UART_PARITY_EVEN
#define SBUS_STOP_BITS 2


#define SBUS_STARTBYTE	0xF0
#define SBUS_ENDBYTE	0X00
#define SBUS_MESSAGE_MAX_SIZE 25

#define SBUS_FIFO_SIZE	4

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

static int chars_rxed = 0;

volatile uint8_t sbus_data[SBUS_FIFO_SIZE][SBUS_MESSAGE_MAX_SIZE];
volatile uint8_t current_sbus_data[SBUS_MESSAGE_MAX_SIZE];
volatile uint8_t oldest = 0;
volatile uint8_t newest = 0;
volatile uint8_t stored = 0;
volatile int sbus_index = 0;
volatile bool hasStartByte = false;


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
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
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
}

int main() {
    stdio_init_all();
    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 115200);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int actual = uart_set_baudrate(UART_ID, SBUS_BAUD_RATE);

    printf("Actual baud rate: %i\n", actual);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, SBUS_DATA_BITS, SBUS_STOP_BITS, SBUS_PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    uart_puts(UART_ID, "\nHello, uart interrupts\n");

    uint8_t sbusData[SBUS_MESSAGE_MAX_SIZE];

    while (1)
    {
        if(hasSbusData())
        {
            readSbusData(sbusData);
            printf("We got data!\n");
        }
        tight_loop_contents();
    }
}

/// \end:uart_advanced[]
