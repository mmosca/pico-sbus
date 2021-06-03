#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "sbus.h"

extern volatile int irq_count;

extern volatile uint8_t oldest;
extern volatile uint8_t newest;
extern volatile uint8_t stored;


int main() {
    stdio_init_all();
    sbus_init();
    // Set up our UART with a basic baud rate.
    uart_init(SBUS_UART_ID, 115200);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);


    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int actual = uart_set_baudrate(SBUS_UART_ID, SBUS_BAUD_RATE);

    printf("Actual baud rate: %i\n", actual);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(SBUS_UART_ID, false, false);

    // Set our data format
    uart_set_format(SBUS_UART_ID, SBUS_DATA_BITS, SBUS_STOP_BITS, SBUS_PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(SBUS_UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = SBUS_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, sbus_on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(SBUS_UART_ID, true, false);

    uint8_t sbusData[SBUS_MESSAGE_MAX_SIZE] = {};

    while (1)
    {
        if(hasSbusData())
        {
            if(readSbusData(sbusData))
            {
                sbus_state_t sbus = {};
                gpio_put(PICO_DEFAULT_LED_PIN, 1);
                memset(&sbus, 0, sizeof(sbus_state_t));
                decode_sbus_data(sbusData, &sbus);

                printf("\033[23");
                printf("\033[H");

                printf("Ch1 : %u\n", sbus.ch1);
                printf("Ch2 : %u\n", sbus.ch2);
                printf("Ch3 : %u\n", sbus.ch3);
                printf("Ch4 : %u\n", sbus.ch4);
                printf("Ch5 : %u\n", sbus.ch5);
                printf("Ch6 : %u\n", sbus.ch6);
                printf("Ch7 : %u\n", sbus.ch7);
                printf("Ch8 : %u\n", sbus.ch8);
                printf("Ch9 : %u\n", sbus.ch9);
                printf("Ch10: %u\n", sbus.ch10);
                printf("Ch11: %u\n", sbus.ch11);
                printf("Ch12: %u\n", sbus.ch12);
                printf("Ch13: %u\n", sbus.ch13);
                printf("Ch14: %u\n", sbus.ch14);
                printf("Ch15: %u\n", sbus.ch15);
                printf("Ch16: %u\n", sbus.ch16);
                printf("Ch17: %u\n", sbus.dch17);
                printf("Ch18: %u\n", sbus.dch18);
                printf("Frame lost: %i Failsafe: %i\n", sbus.framelost, sbus.failsafe);
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
            }

            sleep_ms(500);
        }
        else
        {
            //printf("%i BPS: %i No data! oldest: %i newest: %i\n", irq_count, actual, oldest, newest);
        }
        tight_loop_contents();
    }
}
