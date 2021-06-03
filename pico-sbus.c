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
    sbus_init(SBUS_UART_ID, UART_RX_PIN, UART_TX_PIN);

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

                printf("\033[23");
                printf("\033[H");

                decode_sbus_data(sbusData, &sbus);


                printf("Ch1 : %04u\n", sbus.ch1);
                printf("Ch2 : %04u\n", sbus.ch2);
                printf("Ch3 : %04u\n", sbus.ch3);
                printf("Ch4 : %04u\n", sbus.ch4);
                printf("Ch5 : %04u\n", sbus.ch5);
                printf("Ch6 : %04u\n", sbus.ch6);
                printf("Ch7 : %04u\n", sbus.ch7);
                printf("Ch8 : %04u\n", sbus.ch8);
                printf("Ch9 : %04u\n", sbus.ch9);
                printf("Ch10: %04u\n", sbus.ch10);
                printf("Ch11: %04u\n", sbus.ch11);
                printf("Ch12: %04u\n", sbus.ch12);
                printf("Ch13: %04u\n", sbus.ch13);
                printf("Ch14: %04u\n", sbus.ch14);
                printf("Ch15: %04u\n", sbus.ch15);
                printf("Ch16: %04u\n", sbus.ch16);
                printf("Ch17: %04u\n", sbus.dch17);
                printf("Ch18: %04u\n", sbus.dch18);
                printf("Frame lost: %i Failsafe: %i\n", sbus.framelost, sbus.failsafe);
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
            }

            sleep_ms(10);
        }
        else
        {
            //printf("%i BPS: %i No data! oldest: %i newest: %i\n", irq_count, actual, oldest, newest);
        }
        tight_loop_contents();
    }
}
