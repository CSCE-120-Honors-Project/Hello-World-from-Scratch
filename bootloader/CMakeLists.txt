cmake_minimum_required(VERSION 3.15)
project(bootloader C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build a small bootloader binary from assembly and C
add_executable(${PROJECT_NAME} start.s main.c)

# Place output name on disk as bootloader.elf
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "bootloader.elf")

# Link bootloader against uart, vio, and fat libraries
target_link_libraries(${PROJECT_NAME} PRIVATE uart vio fat)

# Include directories for uart, vio, fat headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/uart
    ${CMAKE_SOURCE_DIR}/filesystem/vio
    ${CMAKE_SOURCE_DIR}/filesystem/fat
)

# Linker: use bootloader's own linker script and no standard libraries
target_link_options(${PROJECT_NAME} PRIVATE "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld" "-nostdlib")

