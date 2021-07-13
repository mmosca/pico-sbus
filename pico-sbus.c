#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/pio.h"

#include "sbus.h"

#include "hardware/clocks.h"

#include "sbus-hid.h"

#include "uart_rx.pio.h"

int main() {
    stdio_init_all();

    // Initialize SBUS and setup interrupt to read data on core0
#ifndef USE_PIO
    sbus_init(SBUS_UART_ID, UART_RX_PIN, UART_TX_PIN);
#else
    //sbus_pio_init(pio0, 9, 10);
    sbus_pio_init_alt(pio0, 9);
#endif
    
    hid_init();

    // Proccess SBUS data and generate HID reports from core1
    multicore_launch_core1(hid_main);

    while (1)
    {
    #ifdef USE_PIO
        printf("irq_count: %i\n", irq_count);
        //sbus_on_uart_rx_alt();
        //if(uart_rx_is_readable(sbus_pio, sm))
        {

        }
    #else
        tight_loop_contents();
    #endif
    }
}
