cmake_minimum_required(VERSION 3.13)
#include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake) # old sdk
include(${PICO_SDK_PATH}/pico_sdk_init.cmake) # new sdk

project(Modbus_example C CXX ASM)

pico_sdk_init()

add_executable(Modbus_example
        src/sendframe.c
        )

target_link_libraries(Modbus_example
    pico_stdlib
    hardware_uart
)
pico_add_extra_outputs(Modbus_example)

enable_stdio_usb(Modbus_example 1)
enable_stdio_uart(Modbus_example 0)