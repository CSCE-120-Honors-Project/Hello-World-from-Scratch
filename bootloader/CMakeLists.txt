cmake_minimum_required(VERSION 3.15)
project(bootloader C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build a small bootloader binary from assembly and C
add_executable(${PROJECT_NAME} start.s main.c)

# Place output name on disk as bootloader.elf
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "bootloader.elf")

# No special link steps here; root CMake provides linker flags if needed

# Linker: use root linker script and no standard libraries
target_link_options(${PROJECT_NAME} PRIVATE "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld" "-nostdlib")
