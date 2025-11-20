cmake_minimum_required(VERSION 4.1.2)
project(bootloader C ASM)  # Add ASM language support

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# AUTO-DETECT ARM64 CROSS-COMPILER TOOLCHAIN
# ============================================================

# Try to find the cross-compiler (looks for any of these)
find_program(CROSS_GCC 
    NAMES 
        aarch64-linux-gnu-gcc           # Linux (Ubuntu/Debian)
        aarch64-none-linux-gnu-gcc      # ARM official / Windows
        aarch64-unknown-linux-gnu-gcc   # macOS Homebrew
)

# If not found, show error with helpful message
if(NOT CROSS_GCC)
    message(FATAL_ERROR "ARM64 cross-compiler not found! See README for installation.")
endif()

# Extract the prefix (everything before "-gcc")
get_filename_component(CROSS_GCC_NAME ${CROSS_GCC} NAME_WE)
string(REGEX REPLACE "-gcc$" "" CROSS_PREFIX ${CROSS_GCC_NAME})

# Show what we found
message(STATUS "Found ARM64 toolchain: ${CROSS_GCC}")
message(STATUS "Using toolchain prefix: ${CROSS_PREFIX}")

# Set all compiler tools
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER ${CROSS_PREFIX}-gcc)
set(CMAKE_ASM_COMPILER ${CROSS_PREFIX}-as)

# Store other tools as variables
set(CROSS_LD ${CROSS_PREFIX}-ld)
set(CROSS_OBJCOPY ${CROSS_PREFIX}-objcopy)
set(CROSS_OBJDUMP ${CROSS_PREFIX}-objdump)
set(CROSS_SIZE ${CROSS_PREFIX}-size)

# Compiler flags for bare metal
set(CMAKE_C_FLAGS "-ffreestanding -nostdlib -O2 -Wall -Wextra")
set(CMAKE_ASM_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -nostdlib")

# Disable standard libraries
set(CMAKE_C_STANDARD_LIBRARIES "")

# ============================================================
# BUILD COMPONENTS
# ============================================================

add_subdirectory(filesystem/vio)
add_subdirectory(filesystem/fat)
add_subdirectory(uart)  # If you have uart
add_subdirectory(os)    # If you have os

# Bootloader executable
add_executable(${PROJECT_NAME} placeholder.c)

target_link_libraries(${PROJECT_NAME} 
    fat
    vio
)

# cmake_minimum_required(VERSION 4.1.2)
# project(bootloader)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add_subdirectory(filesystem/vio)
# add_subdirectory(filesystem/fat)

# # Change placeholder.c to whatever actual bootloader program is going to be run
# add_executable(${PROJECT_NAME} placeholder.c)

# target_link_libraries(${PROJECT_NAME} PUBLIC fat)
# target_link_libraries(${PROJECT_NAME} PUBLIC vio)
