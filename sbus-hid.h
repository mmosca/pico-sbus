#ifndef __SBUS_HID_H__
#define __SBUS_HID_H__

#include <stdbool.h>

#define SBUS_HID_MAX_BUTTONS 31

#define SBUS_HID_MIN 0
#define SBUS_HID_MAX 0x7FF

#define SBUS_HID_LOW_TH (SBUS_HID_MAX / 3)
#define SBUS_HID_HIGH_TH (SBUS_HID_MAX - SBUS_HID_LOW_TH)

typedef struct {
    uint16_t xl;
    uint16_t yl;

    uint16_t xr;
    uint16_t yr;

    uint16_t z;
    uint16_t z_rot;

    bool buttons[SBUS_HID_MAX_BUTTONS];
} joystick_state_t;

typedef enum joystyick_button {
    BUTTON_LOW = 0,
    BUTTON_MID = 1,
    BUTTON_HIGH = 3
} joystick_button_t;

void hid_init();
void hid_main();

joystick_button_t channel2button(uint16_t channelValue);

#endif