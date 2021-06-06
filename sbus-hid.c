#include <string.h>

#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/unique_id.h"

#include "bsp/board.h"

#include "sbus-hid.h"
#include "sbus.h"
#include "usb_descriptors.h"

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

void hid_task(joystick_state_t *joy);
joystick_button_t channel2button2(uint16_t channelValue);
joystick_button_t channel2button3(uint16_t channelValue);
void setJoyStickButton(joystick_state_t *joystick, int low_button, uint16_t channel_value);
void sbus2joystick(const sbus_state_t *sbus, joystick_state_t *joystick);
static void send_hid_report(uint8_t report_id, joystick_state_t *joy);

int8_t scaleAxis(uint16_t value);

static bool hasData = false;


void hid_init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  // read pico board id for SN
   pico_unique_board_id_t id;
   pico_get_unique_board_id(&id);

   for(int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i)
   {
       snprintf(pico_id_str + (i*2), 3, "%02x", id.id[i]);
   }

   string_desc_arr[3] = pico_id_str;

    board_init();
    tusb_init();
}

void hid_main()
{
    uint8_t sbusData[SBUS_MESSAGE_MAX_SIZE] = {};
    joystick_state_t joy = {};
    int clear_counter = 0;

    while(1)
    {
        tud_task(); // device task

        if(hasSbusData())
        {
            hasData = true;
            /*
            printf("\033[2J");
            printf("\033[H");
             */

            if(readSbusData(sbusData))
            {
                sbus_state_t sbus = {};
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
                memset(&sbus, -1, sizeof(sbus_state_t));


                decode_sbus_data(sbusData, &sbus);
                sbus2joystick(&sbus, &joy);

                for(int i = 0; i < 18; ++i)
                {
                    printf("Ch%2i: %04u\n", i, sbus.ch[i]);
                }
                printf("Frame lost: %i Failsafe: %i\n", sbus.framelost, sbus.failsafe);
                gpio_put(PICO_DEFAULT_LED_PIN, 0);

                printf("Joy L: %i, %i\n", joy.xl, joy.yl);
                printf("Joy R: %i, %i\n", joy.xr, joy.yr);
                printf("Joy Z: %i, %i\n", joy.z, joy.z_rot);

                for (int i = 0; i < SBUS_HID_MAX_BUTTONS; ++i)
                {
                    printf("Button %2i: %i\n", i + 1, joy.buttons[i]);
                }
            }
        }
        else
        {
            hasData = 0;
        }

        hid_task(&joy);
    }
}

void hid_task(joystick_state_t *joy)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time
    start_ms = board_millis();

    // Remote wakeup
    if (tud_suspended() && hasData)
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }
    else
    {
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report(REPORT_ID_GAMEPAD, joy);
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

static void send_hid_report(uint8_t report_id, joystick_state_t *joy)
{
    // skip if hid is not ready yet
    if (!tud_hid_ready())
        return;

    switch (report_id)
    {
        case REPORT_ID_GAMEPAD:
        {
            hid_gamepad_report_t report =
                {
                    .x = scaleAxis(joy->xl), 
                    .y = scaleAxis(joy->yl), 
                    .z = scaleAxis(joy->z), 
                    .rz = scaleAxis(joy->z_rot), 
                    .rx = scaleAxis(joy->xr), 
                    .ry = scaleAxis(joy->yr), 
                    .hat = 0, 
                    .buttons = 0
                };

            for (int i = 0; i < SBUS_HID_MAX_BUTTONS && i < 16; ++i)
            {
                if (joy->buttons[i])
                {
                    report.buttons |= (1 << i);
                }
            }
            tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        }
        break;

        default:
            break;
    }
}

joystick_button_t channel2button3(uint16_t channelValue)
{
    if(channelValue < SBUS_HID_LOW_TH)
    {
        return BUTTON_LOW;
    } else if(channelValue > SBUS_HID_HIGH_TH)
    {
        return BUTTON_HIGH;
    }

    return BUTTON_MID;
}

joystick_button_t channel2button2(uint16_t channelValue)
{
    if(channelValue < SBUS_HID_MID_TH)
    {
        return BUTTON_LOW;
    }

    return BUTTON_MID;
}

void setJoyStickButton2(joystick_state_t *joystick, int low_button, uint16_t channel_value)
{
    int b = channel2button2(channel_value);
    joystick->buttons[low_button + b] = true;
}

void setJoyStickButton3(joystick_state_t *joystick, int low_button, uint16_t channel_value)
{
    int b = channel2button3(channel_value);
    joystick->buttons[low_button + b] = true;
}

void sbus2joystick(const sbus_state_t *sbus, joystick_state_t *joystick)
{
    memset(joystick, 0, sizeof(joystick_state_t));

    joystick->xr = sbus->ch[0];
    joystick->yr = sbus->ch[1];
    joystick->yl = sbus->ch[2];
    joystick->xl = sbus->ch[3];
    joystick->z = sbus->ch[5];
    joystick->z_rot = sbus->ch[7];

    for (int i = 0; axis_button_mapping[i].channel != -1; ++i)
    {
        setJoyStickButton2(joystick, axis_button_mapping[i].first_button, sbus->ch[axis_button_mapping[i].channel]);
    }

    if (sbus->ch[16] > SBUS_HID_HIGH_TH)
    {
        joystick->buttons[14] = true;
    }
    if (sbus->ch[17] > SBUS_HID_HIGH_TH)
    {
        joystick->buttons[15] = true;
    }
}


int8_t scaleAxis(uint16_t value)
{
    // Just drop lower bits 255-0
    int16_t smallerValue = value >> 3;

    return smallerValue - 127;
}
