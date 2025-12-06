cmake_minimum_required(VERSION 3.15)

# Set toolchain BEFORE project()
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/arm64-toolchain.cmake)

project(BareMetal_System C ASM)

# ============================================================
# BUILD CONFIGURATION
# ============================================================

# Export compile commands for IDEs (VSCode, CLion, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Make build verbose by default (show all commands)
set(CMAKE_VERBOSE_MAKEFILE ON)

# ============================================================
# TOOLCHAIN AUTO-DETECTION
# ============================================================

# Try to find ARM64 cross-compiler
find_program(CROSS_GCC 
    NAMES 
        aarch64-elf-gcc                 # macOS Homebrew
        aarch64-linux-gnu-gcc           # Linux (Ubuntu/Debian)
        aarch64-none-linux-gnu-gcc      # ARM official / Windows
        aarch64-unknown-linux-gnu-gcc   # Alternative macOS
    DOC "ARM64 cross-compiler"
)

# Error if not found
if(NOT CROSS_GCC)
    message(FATAL_ERROR 
        "ARM64 cross-compiler not found!\n"
        "Install instructions:\n"
        "  Linux:   sudo apt install gcc-aarch64-linux-gnu\n"
        "  Windows: Download from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads\n"
        "  macOS:   brew install aarch64-elf-gcc"
    )
endif()

# Extract toolchain prefix (e.g., "aarch64-linux-gnu")
get_filename_component(CROSS_GCC_NAME ${CROSS_GCC} NAME_WE)
string(REGEX REPLACE "-gcc$" "" CROSS_PREFIX ${CROSS_GCC_NAME})

# Show what we found
message(STATUS "Found ARM64 toolchain: ${CROSS_GCC}")
message(STATUS "Using toolchain prefix: ${CROSS_PREFIX}")

# ============================================================
# TOOLCHAIN CONFIGURATION
# ============================================================

# Set cross-compilation target
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Set compilers
if(NOT DEFINED CMAKE_C_COMPILER OR CMAKE_C_COMPILER STREQUAL "")
    set(CMAKE_C_COMPILER ${CROSS_PREFIX}-gcc)
endif()
if(NOT DEFINED CMAKE_ASM_COMPILER OR CMAKE_ASM_COMPILER STREQUAL "")
    set(CMAKE_ASM_COMPILER ${CROSS_PREFIX}-gcc)
endif()
if(NOT DEFINED CMAKE_CXX_COMPILER OR CMAKE_CXX_COMPILER STREQUAL "")
    set(CMAKE_CXX_COMPILER ${CROSS_PREFIX}-g++)
endif()

# Store other tools as variables
set(CROSS_LD ${CROSS_PREFIX}-ld)
set(CROSS_OBJCOPY ${CROSS_PREFIX}-objcopy)
set(CROSS_OBJDUMP ${CROSS_PREFIX}-objdump)
set(CROSS_SIZE ${CROSS_PREFIX}-size)

# ============================================================
# COMPILER FLAGS
# ============================================================

# C compiler flags for bare metal
set(CMAKE_C_FLAGS "-ffreestanding -nostdlib -O2 -Wall -Wextra")

# Assembly flags
set(CMAKE_ASM_FLAGS "")

# Linker flags (will be overridden by individual linker scripts)
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")

# Disable standard libraries
set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES "")

# ============================================================
# BUILD COMPONENTS
# ============================================================

# Build libraries first (in order of dependencies)
add_subdirectory(uart)
add_subdirectory(filesystem/vio)
add_subdirectory(filesystem/fat)

# Build bootloader
add_subdirectory(bootloader)

# Build OS
add_subdirectory(os)

# ============================================================
# BUILD SUMMARY
# ============================================================

message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "  Toolchain:  ${CROSS_PREFIX}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Assembler:  ${CMAKE_ASM_COMPILER}")
message(STATUS "  C Flags:    ${CMAKE_C_FLAGS}")
message(STATUS "")
message(STATUS "  Components:")
message(STATUS "    - UART library")
message(STATUS "    - VIO library")
message(STATUS "    - FAT library")
message(STATUS "    - Bootloader (firmware.elf)")
message(STATUS "    - OS Kernel (kernel.elf)")
message(STATUS "")
