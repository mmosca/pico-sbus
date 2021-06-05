#include <string.h>

#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bsp/board.h"

#include "sbus-hid.h"
#include "sbus.h"

struct axis_button_mapping_t {
   int channel;
   int first_button; 
} axis_button_mapping[] = {
    {.channel = 4, .first_button = 0},
    {6, 2},
    {8, 5},
    {9, 8},
    {10, 11},
    {11, 14},
    {12, 17},
    {13, 20},
    {14, 23},
    {15, 26},
    {-1, -1}};

joystick_button_t channel2button(uint16_t channelValue)
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

void setJoyStickButton(joystick_state_t *joystick, int low_button, uint16_t channel_value)
{
    int b = channel2button(channel_value);
    joystick->buttons[low_button + b] = true;
}

void sbus2joystick(const sbus_state_t *sbus, joystick_state_t *joystick)
{
    memset(joystick, 0, sizeof(joystick_state_t));

    joystick->yr = sbus->ch[0];
    joystick->xr = sbus->ch[1];
    joystick->yl = sbus->ch[2];
    joystick->xl = sbus->ch[3];
    joystick->z = sbus->ch[5];
    joystick->z_rot = sbus->ch[7];

    for (int i = 0; axis_button_mapping[i].channel != -1; ++i)
    {
        setJoyStickButton(joystick, axis_button_mapping[i].first_button, sbus->ch[axis_button_mapping[i].channel]);
    }

    if (sbus->ch[16] > SBUS_HID_HIGH_TH)
    {
        joystick->buttons[29] = true;
    }
    if (sbus->ch[17] > SBUS_HID_HIGH_TH)
    {
        joystick->buttons[30] = true;
    }
}

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
    joystick_state_t joy = {};
    int clear_counter = 0;

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

            sleep_ms(9);
        }
        else
        {
            //printf("%i BPS: %i No data! oldest: %i newest: %i\n", irq_count, actual, oldest, newest);
        }


        tight_loop_contents();
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
