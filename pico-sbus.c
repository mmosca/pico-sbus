#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/pio.h"

#include "sbus.h"
#include "sbus-hid.h"

int main() {
    stdio_init_all();

    // Initialize SBUS and setup interrupt to read data on core0
    //sbus_init(SBUS_UART_ID, UART_RX_PIN, UART_TX_PIN);

    sbus_pio_init(pio0, 9, 10);
    
    hid_init();

    // Proccess SBUS data and generate HID reports from core1
    multicore_launch_core1(hid_main);

    while (1)
    {
        tight_loop_contents();
    }
}
