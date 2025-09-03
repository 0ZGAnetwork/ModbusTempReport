cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(Mgs_project_CICD_TDD C CXX ASM)
pico_sdk_init()
# program
add_executable(Mgs_project_CICD_TDD
        scr/blink.c
        )
target_link_libraries(Mgs_project_CICD_TDD pico_stdlib)
pico_add_extra_outputs(Mgs_project_CICD_TDD)