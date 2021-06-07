#include "input-mapping.h"

#include <string.h>

void get_input_map(input_map_t *map)
{
    memset(map, 0, sizeof(input_map_t));

    map->lx = INPUT_DEFAULT_XL;
    map->ly = INPUT_DEFAULT_YL;

    map->rx = INPUT_DEFAULT_XR;
    map->ry = INPUT_DEFAULT_YR;

    map->z = INPUT_DEFAULT_Z;
    map->rz = INPUT_DEFAULT_ZR;

    int bttn=0;

    map->button_map[bttn].channel = 4;
    map->button_map[bttn++].type = BUTTON_2POS_LOW;

    map->button_map[bttn].channel = 4;
    map->button_map[bttn++].type = BUTTON_2POS_HIGH;

    map->button_map[bttn].channel = 6;
    map->button_map[bttn++].type = BUTTON_2POS_LOW;

    map->button_map[bttn].channel = 6;
    map->button_map[bttn++].type = BUTTON_2POS_HIGH;

    // last 2 channels are the digital channels
    for(int channel = 8; bttn < INPUT_MAX_BUTTONS - 2; channel++)
    {
        map->button_map[bttn].channel = channel;
        map->button_map[bttn++].type = BUTTON_2POS_LOW;

        map->button_map[bttn].channel = channel;
        map->button_map[bttn++].type = BUTTON_2POS_HIGH;
    }

    map->button_map[bttn].channel = 16;
    map->button_map[bttn++].type = BUTTON_2POS_HIGH;

    map->button_map[bttn].channel = 17;
    map->button_map[bttn++].type = BUTTON_2POS_HIGH;
}

uint8_t getAxisFromSbus(const sbus_state_t *sbus, int channel)
{
    int16_t smallerValue = sbus->ch[channel] >> 3;

    return smallerValue - 127;
}

bool isPressed(const sbus_state_t *sbus, const input_button_mapping_item_t *map)
{
    uint16_t rawValue = sbus->ch[map->channel];

    switch(map->type)
    {
        case BUTTON_2POS_LOW:
            return rawValue < 1024;
        case BUTTON_2POS_HIGH:
            return rawValue >= 1024;
        case BUTTON_3POS_LOW:
            return rawValue < 682;
        case BUTTON_3POS_MID:
            return rawValue >= 682 && rawValue < 1365;
        case BUTTON_3POS_HIGH:
            return rawValue >= 1365;
    }

    return false;
}

void sbus2gamepad_report(const input_map_t *map, const sbus_state_t *sbus, hid_gamepad_report_t *hid)
{

}
