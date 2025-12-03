cmake_minimum_required(VERSION 3.15)
project(os C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# creates os executable

# creates os executable
add_executable(${PROJECT_NAME} main.c start.s)

# Ensure the OS is linked with its own linker script (defines boot_stack_top, bss symbols)
target_link_options(${PROJECT_NAME} PRIVATE "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld" "-nostdlib")

# Rename the physical output file (Optional but recommended for embedded)
# This keeps the target name "os" inside CMake, but creates "os.elf" on disk.
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "os.elf")

# link the libraries
target_link_libraries(${PROJECT_NAME} uart)

# include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# create binary
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CROSS_OBJCOPY} -O binary # using variable objcopy because it varies by host operating system
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_BINARY_DIR}/os.bin
    COMMENT "Creating os.bin from os.elf"
)