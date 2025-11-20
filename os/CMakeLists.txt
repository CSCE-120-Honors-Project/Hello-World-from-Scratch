cmake_minimum_required(VERSION 4.1.2)
project(os)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# creates os executable
add_executable(${PROJECT_NAME} main.c start.s)

# link the libraries
target_link_libraries(os.elf uart)

# include directories
target_include_directories(os.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# create binary
add_custom_command(TARGET os.elf POST_BUILD
    COMMAND ${OBJCOPY} -O binary # using variable objcopy because it varies by host operating system
            $<TARGET_FILE:os.elf>
            ${CMAKE_BINARY_DIR}/os.bin
    COMMENT "Creating os.bin"
)