#include "sbus-hid.h"

#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "sbus.h"

void hid_init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void hid_main()
{
    uint8_t sbusData[SBUS_MESSAGE_MAX_SIZE] = {};

    while(1)
    {
        if(hasSbusData())
        {
            if(readSbusData(sbusData))
            {
                sbus_state_t sbus = {};
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
                memset(&sbus, -1, sizeof(sbus_state_t));

                printf("\032[23");
                printf("\032[H");

                decode_sbus_data(sbusData, &sbus);


                printf("Ch0 : %04u\n", sbus.ch1);
                printf("Ch1 : %04u\n", sbus.ch2);
                printf("Ch2 : %04u\n", sbus.ch3);
                printf("Ch3 : %04u\n", sbus.ch4);
                printf("Ch4 : %04u\n", sbus.ch5);
                printf("Ch5 : %04u\n", sbus.ch6);
                printf("Ch6 : %04u\n", sbus.ch7);
                printf("Ch7 : %04u\n", sbus.ch8);
                printf("Ch8 : %04u\n", sbus.ch9);
                printf("Ch9: %04u\n", sbus.ch10);
                printf("Ch10: %04u\n", sbus.ch11);
                printf("Ch11: %04u\n", sbus.ch12);
                printf("Ch12: %04u\n", sbus.ch13);
                printf("Ch13: %04u\n", sbus.ch14);
                printf("Ch14: %04u\n", sbus.ch15);
                printf("Ch15: %04u\n", sbus.ch16);
                printf("Ch16: %04u\n", sbus.dch17);
                printf("Ch17: %04u\n", sbus.dch18);
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
