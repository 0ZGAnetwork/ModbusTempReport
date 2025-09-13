cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(Mgs_project_CICD_TDD C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# cmake -DPICO_BOARD=pico2 ..
set(PICO_BOARD pico2)
pico_sdk_init()

add_executable(Mgs_project_CICD_TDD
        src/blink.c
        )
target_link_libraries(Mgs_project_CICD_TDD 
        pico_stdlib
        pico_stdio_usb
        )
pico_add_extra_outputs(Mgs_project_CICD_TDD)

pico_enable_stdio_usb(Mgs_project_CICD_TDD 1)
pico_enable_stdio_uart(Mgs_project_CICD_TDD 0)