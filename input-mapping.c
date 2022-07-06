#include "input-mapping.h"

#include <string.h>

#if INPUT_MAX_BUTTONS > 16
  #define B32(X) (X)
#else
  #define B32(X)
#endif

void get_input_map(input_map_t *map)
{
    memset(map, 0, sizeof(input_map_t));

    map->lx = INPUT_DEFAULT_XL;
    map->ly = INPUT_DEFAULT_YL;

    map->rx = INPUT_DEFAULT_XR;
    map->ry = INPUT_DEFAULT_YR;

    map->ls = INPUT_DEFAULT_LS;
    map->rs = INPUT_DEFAULT_RS;

    map->ld = INPUT_DEFAULT_LD;
    map->rd = INPUT_DEFAULT_RD;

    int bttn=0;

    map->button_map[bttn].channel = 4;
    map->button_map[bttn++].type = BUTTON_3POS_LOW;

    map->button_map[bttn].channel = 4;
    map->button_map[bttn++].type = BUTTON_3POS_MID;

    map->button_map[bttn].channel = 4;
    map->button_map[bttn++].type = BUTTON_3POS_HIGH;

    // last 2 channels are the digital channels
    for (int channel = 9; bttn < (INPUT_MAX_BUTTONS - 2) && channel < (SBUS_CHANNEL_COUNT - 2); channel++)
    {
        map->button_map[bttn].channel = channel;
        map->button_map[bttn++].type = BUTTON_3POS_LOW;

        map->button_map[bttn].channel = channel;
        map->button_map[bttn++].type = BUTTON_3POS_MID;

        map->button_map[bttn].channel = channel;
        map->button_map[bttn++].type = BUTTON_3POS_HIGH;
    }

    map->button_map[bttn].channel = SBUS_CHANNEL_COUNT - 2;
    map->button_map[bttn++].type = BUTTON_2POS_HIGH;

    map->button_map[bttn].channel = SBUS_CHANNEL_COUNT -1;
    map->button_map[bttn++].type = BUTTON_2POS_HIGH;
}


bool parse_input_map(uint8_t *data, size_t data_size, input_map_t *newMap)
{
    if(data_size != sizeof(input_map_t))
    {
        get_input_map(newMap);
    }

    memcpy(newMap, data, sizeof(input_map_t));
}

int16_t getAxisFromSbus(const sbus_state_t *sbus, int channel)
{
    //int16_t smallerValue = sbus->ch[channel] >> 3;

    //return smallerValue - 127;
    return sbus->ch[channel];
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
        case BUTTON_NONE:
            return false;
    }

    return false;
}

void sbus2gamepad_report(const input_map_t *map, const sbus_state_t *sbus, hid_rcgamepad_report_t *hid)
{

}
