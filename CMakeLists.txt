cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(pico-sbus)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(pico-sbus
	pico-sbus.c
    sbus.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(pico-sbus 1)
pico_enable_stdio_uart(pico-sbus 0)

target_link_libraries(pico-sbus pico_stdlib)

pico_add_extra_outputs(pico-sbus)
