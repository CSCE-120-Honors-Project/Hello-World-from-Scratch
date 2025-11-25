cmake_minimum_required(VERSION 4.1.2)
project(os)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# creates os executable
add_executable(${PROJECT_NAME} main.c start.s)

# Rename the physical output file (Optional but recommended for embedded)
# This keeps the target name "os" inside CMake, but creates "os.elf" on disk.
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "os.elf")

# link the libraries
target_link_libraries(${PROJECT_NAME} uart)

# include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# create binary
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${OBJCOPY} -O binary # using variable objcopy because it varies by host operating system
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_BINARY_DIR}/os.bin
    COMMENT "Creating os.bin from os.elf"
)