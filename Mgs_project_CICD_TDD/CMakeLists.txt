cmake_minimum_required(VERSION 3.13)

project(Mgs_project_CICD_TDD C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# --- Firmware for Pico ---
if(NOT HOST_BUILD)
        include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
                #cmake -DPICO_BOARD=pico2 ..
        set(PICO_BOARD pico2)
        pico_sdk_init()

        # --- Application ---
        add_executable(Mgs_project_CICD_TDD
                src/blink.c
                )

        pico_enable_stdio_usb(Mgs_project_CICD_TDD 1)
        pico_enable_stdio_uart(Mgs_project_CICD_TDD 0)
        target_link_libraries(Mgs_project_CICD_TDD 
                pico_stdlib
                )
        pico_add_extra_outputs(Mgs_project_CICD_TDD)
endif()

# --- Local Testing (Host) ---
if(HOST_BUILD)
        enable_testing()

        add_executable(unit_test test/unit_test.c)
        add_test( NAME unit_test COMMAND unit_test)

        add_executable(integration_test test/integration_test.c)
        add_test( NAME integration_test COMMAND integration_test)
endif()