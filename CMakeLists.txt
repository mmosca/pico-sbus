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
    sbus/sbus.c
    sbus-hid.c
)

#add_compile_definitions(PICO_STDOUT_MUTEX)

target_include_directories(pico-sbus PRIVATE ${CMAKE_CURRENT_LIST_DIR} . sbus)

# Stdio output usb/sbus
pico_enable_stdio_usb(pico-sbus 0)
pico_enable_stdio_uart(pico-sbus 1)

target_link_libraries(pico-sbus 
    pico_stdlib
    pico_multicore
    tinyusb_device
    tinyusb_board)

pico_add_extra_outputs(pico-sbus)
