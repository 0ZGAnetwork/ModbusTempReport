cmake_minimum_required(VERSION 3.13)
#include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake) # old sdk
#set(PICO_SDK_PATH "C:/pico-sdk")

# include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake) # new sdk
# include(pico_sdk_import.cmake)
# set(PICO_SDK_PATH "/home/pawel/pico-sdk")

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
message(STATUS "ENV PICO_SDK_PATH = $ENV{PICO_SDK_PATH}")

project(Modbus_example C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()


set(PROJECT_NAME Modbus_example)

add_executable(${PROJECT_NAME}
        src/driver_control.c
        src/driver.c
        src/main.c
        #src/sdc35_status.c
        #src/sdc35_csv.c
        #src/modbus_test.c
        #test/test.c
        )

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_uart
    # hardware_rtc
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_STDIO_FLOAT_SUPPORT=1
)

pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)