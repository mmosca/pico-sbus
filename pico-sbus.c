#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "sbus.h"
#include "sbus-hid.h"

extern volatile int irq_count;

extern volatile uint8_t oldest;
extern volatile uint8_t newest;
extern volatile uint8_t stored;


int main() {
    stdio_init_all();
    sbus_init(SBUS_UART_ID, UART_RX_PIN, UART_TX_PIN);
    hid_init();

    multicore_launch_core1(hid_main);

    while (1)
    {

        tight_loop_contents();
    }
}
