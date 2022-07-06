#ifndef _RC_JOY_HID_H_
#define _RC_JOY_HID_H_

#include "tusb.h"

#define TUD_HID_REPORT_DESC_RCGAMEPAD(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                 ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  )                 ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                 ,\
    /* Report ID if any */\
    __VA_ARGS__ \
    /* 12 bit X, Y, Z, Rz, Rx, Ry (min 0, max 2048 ) */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP                 ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_X                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_Y                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RX                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RY                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_Z                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RZ                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_SLIDER               ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_DIAL                 ) ,\
    HID_LOGICAL_MIN_N  ( 0, 2                                   ) ,\
    HID_LOGICAL_MAX_N  ( (int16_t)2048, 2                       ) ,\
    HID_REPORT_COUNT   ( 8                                      ) ,\
    HID_REPORT_SIZE    ( 16                                      ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
    /* 32 bit Button Map */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_BUTTON                  ) ,\
    HID_USAGE_MIN      ( 1                                      ) ,\
    HID_USAGE_MAX      ( 32                                     ) ,\
    HID_LOGICAL_MIN    ( 0                                      ) ,\
    HID_LOGICAL_MAX    ( 1                                      ) ,\
    HID_REPORT_COUNT   ( 32                                     ) ,\
    HID_REPORT_SIZE    ( 1                                      ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
  HID_COLLECTION_END \

typedef struct TU_ATTR_PACKED
{
	uint16_t x;
	uint16_t y;
	uint16_t rx;
	uint16_t ry;
	uint16_t ls;
	uint16_t rs;
	uint16_t ld;
	uint16_t rd;
	uint32_t buttons;
} hid_rcgamepad_report_t;

#endif
