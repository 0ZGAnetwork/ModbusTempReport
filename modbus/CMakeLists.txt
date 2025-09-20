cmake_minimum_required(VERSION 3.13)
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Modbus_example)

pico_sdk_init()

add_executable(Modbus_example
        src/sendframe.c
        )

target_link_libraries(Modbus_example
    pico_stdlib
    hardware_uart
)
pico_add_extra_outputs(Modbus_example)