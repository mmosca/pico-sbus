#include "sbus-hid.h"

#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bsp/board.h"

#include "sbus.h"

void hid_init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    board_init();
    tusb_init();
}

void hid_main()
{
    uint8_t sbusData[SBUS_MESSAGE_MAX_SIZE] = {};
    int clear_counter = 0;

    while(1)
    {
#ifdef USB_HID
        tud_task(); // device task
        //tuh_task(); // host task
#endif
        if ((clear_counter++) % 80000)
        {
            printf("\033[2J");
        }

        if(hasSbusData())
        {
            printf("\033[H");

            if(readSbusData(sbusData))
            {
                sbus_state_t sbus = {};
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
                memset(&sbus, -1, sizeof(sbus_state_t));


                decode_sbus_data(sbusData, &sbus);


                for(int i = 0; i < 18; ++i)
                {
                    printf("Ch%2i: %04u\n", i, sbus.ch[i]);
                }
                printf("Frame lost: %i Failsafe: %i\n", sbus.framelost, sbus.failsafe);
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
            }

            sleep_ms(9);
        }
        else
        {
            //printf("%i BPS: %i No data! oldest: %i newest: %i\n", irq_count, actual, oldest, newest);
        }


        tight_loop_contents();
    }
}

#ifdef USB_HID
//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

void hid_task(void) {
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms) return; // not enough time
    start_ms += interval_ms;

    uint32_t const btn = 1;

    // Remote wakeup
    if (tud_suspended() && btn) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    /*------------- Mouse -------------*/
    if (tud_hid_ready()) {
        if (btn) {
            int8_t const delta = 5;

            // no button, right + down, no scroll pan
            tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);

            // delay a bit before attempt to send keyboard report
            board_delay(10);
        }
    }

    /*------------- Keyboard -------------*/
    if (tud_hid_ready()) {
        // use to avoid send multiple consecutive zero report for keyboard
        static bool has_key = false;

        static bool toggle = false;
        if (toggle = !toggle) {
            uint8_t keycode[6] = {0};
            keycode[0] = HID_KEY_A;

            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);

            has_key = true;
        } else {
            // send empty key report if previously has key pressed
            if (has_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            has_key = false;
        }
    }
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    // TODO not Implemented
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    // TODO set LED based on CAPLOCK, NUMLOCK etc...
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // Blink every interval ms
    if (board_millis() - start_ms < blink_interval_ms) return; // not enough time
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}
#endif