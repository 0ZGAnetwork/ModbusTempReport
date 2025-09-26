cmake_minimum_required(VERSION 3.13)
#include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake) # old sdk
set(PICO_SDK_PATH "C:/pico-sdk")
include(${PICO_SDK_PATH}/pico_sdk_init.cmake) # new sdk

project(Modbus_example C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(Modbus_example
        src/driver_control.c
        src/driver.c
        src/main.c
        )

target_link_libraries(Modbus_example
    pico_stdlib
    hardware_uart
)
pico_add_extra_outputs(Modbus_example)

enable_stdio_usb(Modbus_example 1)
enable_stdio_uart(Modbus_example 0)