#ifndef __INPUT_MAPPING_H__
#define __INPUT_MAPPING_H__

#include <stdint.h>

#include "tusb.h"
#include "sbus.h"

#define INPUT_MAX_BUTTONS 32

typedef enum input_button_type {
    BUTTON_NONE = 0,
    BUTTON_2POS_LOW,  // 2 POSITION, SBUS VALUE < 1024
    BUTTON_2POS_HIGH, // 2 POSITION, SBUS VALUE >= 1024
    BUTTON_3POS_LOW,  // 3 POSITION, LOWER THIRD < 682
    BUTTON_3POS_MID,  // 3 POSITION, MIDDLER THIRD >= 682 && < 1365
    BUTTON_3POS_HIGH, // 3 POSITION, HIGHER THIRD >= 1365
} input_button_type_t;

typedef struct {
    uint8_t channel;  // 0-17
    input_button_type_t type;
} input_button_mapping_item_t;

#define INPUT_DEFAULT_XL 3
#define INPUT_DEFAULT_YL 2

#define INPUT_DEFAULT_XR 0
#define INPUT_DEFAULT_YR 1

#define INPUT_DEFAULT_Z  5
#define INPUT_DEFAULT_ZR 7

typedef struct __attribute__((__packed__)) {
    // Axis channel numbers 0-17
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint8_t z;
    uint8_t rz;

    input_button_mapping_item_t button_map[INPUT_MAX_BUTTONS];
} input_map_t;

void get_input_map(input_map_t *);

bool parse_input_map(uint8_t *data, size_t data_size, input_map_t *newMap);

uint8_t getAxisFromSbus(const sbus_state_t *sbus, int channel);
bool isPressed(const sbus_state_t *sbus, const input_button_mapping_item_t *map);

void sbus2gamepad_report(const input_map_t *map, const sbus_state_t *sbus, hid_gamepad_report_t *hid);


#endif
