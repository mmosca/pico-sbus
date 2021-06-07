#ifndef __SBUS_HID_H__
#define __SBUS_HID_H__

#include <stdbool.h>

#define SBUS_HID_MAX_BUTTONS 16

#define SBUS_HID_MIN 0
#define SBUS_HID_MAX 0x7FF

#define SBUS_HID_LOW_TH (SBUS_HID_MAX / 3)
#define SBUS_HID_MID_TH (1024)
#define SBUS_HID_HIGH_TH (SBUS_HID_MAX - SBUS_HID_LOW_TH)


void hid_init();
void hid_main();


#endif
