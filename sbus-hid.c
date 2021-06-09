#include <string.h>

#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/unique_id.h"

#include "bsp/board.h"

#include "sbus.h"
#include "sbus-hid.h"
#include "usb_descriptors.h"

#include "input-mapping.h"

static input_map_t input_map = {};
struct axis_button_mapping_t {
   int channel;
   int first_button; 
} axis_button_mapping[] = {
    {4, 0},
    {6, 2},
    {8, 4},
    {9, 6},
    {10, 8},
    {11, 10},
    {12, 12},
    {-1, -1}};

 extern char const* string_desc_arr [];
 static char pico_id_str[(PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2) + 1] ;

void hid_task(const sbus_state_t *sbus);
static void send_hid_report(uint8_t report_id, const sbus_state_t *sbus);

int8_t scaleAxis(uint16_t value);

void hid_init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // read pico board id for SN
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);

    get_input_map(&input_map);

    for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i)
    {
        snprintf(pico_id_str + (i * 2), 3, "%02x", id.id[i]);
    }

    string_desc_arr[3] = pico_id_str;

    board_init();
    tusb_init();
}

void hid_main()
{
    uint8_t sbusData[SBUS_MESSAGE_MAX_SIZE] = {};
    sbus_state_t sbus = {};
    int clear_counter = 0;
    const uint32_t interval_ms = 500;
    uint32_t start_ms = board_millis();

    while(1)
    {
        tud_task(); // device task

        if(hasSbusData())
        {
            /*
            printf("\033[2J");
            printf("\033[H");
             */

            if(readSbusData(sbusData))
            {
                memset(&sbus, -1, sizeof(sbus_state_t));

                decode_sbus_data(sbusData, &sbus);
                gpio_put(PICO_DEFAULT_LED_PIN, !sbus.framelost);

                if (board_millis() - start_ms > interval_ms) {
                    start_ms = board_millis();

                    for(int i = 0; i < 18; ++i)
                    {
                        printf("Ch%2i: %04u\n", i, sbus.ch[i]);
                    }

                    printf("Frame lost: %i Failsafe: %i\n", sbus.framelost, sbus.failsafe);
                }
            }
        }
        else
        {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        hid_task((const sbus_state_t *)&sbus);
        sbus.framelost = 1;
    }
}

void hid_task(const sbus_state_t *sbus)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time
    start_ms = board_millis();

    if(!sbus->framelost)
    {
        // Remote wakeup
        if (tud_suspended())
        {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            tud_remote_wakeup();
        }
        else
        {
            // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
            send_hid_report(REPORT_ID_GAMEPAD, sbus);
        }
    }
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    // TODO set LED based on CAPLOCK, NUMLOCK etc...
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;

    if(report_id == 0 && report_type == 0)
    {
        if(buffer[0] == 0x1F)
        {

        }
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    // TODO not Implemented
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

static void send_hid_report(uint8_t report_id, const sbus_state_t *sbus)
{
    // skip if hid is not ready yet
    if (!tud_hid_ready())
        return;

    switch (report_id)
    {
        case REPORT_ID_GAMEPAD:
        {
            if(!sbus->framelost)
            {
                hid_gamepad_report_t report =
                {
                    .x = getAxisFromSbus(sbus, input_map.lx),
                    .y = getAxisFromSbus(sbus, input_map.ly),
                    .z = getAxisFromSbus(sbus, input_map.z),
                    .rz = getAxisFromSbus(sbus, input_map.rz),
                    .rx = getAxisFromSbus(sbus, input_map.rx),
                    .ry = getAxisFromSbus(sbus, input_map.ry),
                    .hat = 0,
                    .buttons = 0};

                for (int i = 0; i < INPUT_MAX_BUTTONS; ++i)
                {
                    if (isPressed(sbus, &input_map.button_map[i]))
                    {
                        report.buttons |= (1 << i);
                    }
                }
                tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
            }
        }
        break;

        default:
            break;
    }
}

