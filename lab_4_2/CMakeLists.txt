# Verision for Pico Probe debugger making only .elf file and writing it to pico.
# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Set PICO_SDK_PATH
set(PICO_SDK_PATH /home/dek0n/pi/pico/pico-sdk)

# Include build functions from Pico SDK
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(PICO_BOARD pico_w)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(my_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
    main.c
    my_pico_definitions.h
    my_pico_functions.c
)

# Create map/bin/hex/uf2 files
#pico_add_extra_outputs(${PROJECT_NAME})

# Create uf2
# pico_add_uf2_output(${PROJECT_NAME})


# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_pwm
    hardware_i2c
    hardware_uart
    hardware_gpio
    hardware_irq
)

# Disable usb output, enable uart output - CMakeLists version fo Pico Probe debugger
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)


# For writing .elf to pico with openOCD
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program $<TARGET_FILE:${PROJECT_NAME}> verify reset exit"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Programming the target using OpenOCD"
)

#e.g. it works also
# sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program build/lab_4.elf verify reset exit"