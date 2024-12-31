cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(JoybusProbe  C CXX ASM)

set(PICO_BOARD pico)

pico_sdk_init()

add_executable(JoybusProbe
    src/main.c
    src/joybus.c
)

pico_generate_pio_header(JoybusProbe ${CMAKE_CURRENT_LIST_DIR}/src/joybus.pio)

target_link_libraries(JoybusProbe
    hardware_dma
    hardware_pio
    pico_stdlib
)

pico_enable_stdio_usb(JoybusProbe 1)
pico_enable_stdio_uart(JoybusProbe 1)

pico_add_extra_outputs(JoybusProbe)
