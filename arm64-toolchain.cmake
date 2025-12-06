# ARM64 Cross-Compilation Toolchain for QEMU bare-metal
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Find the cross-compiler (try multiple variants for different platforms)
find_program(AARCH64_GCC 
    NAMES 
        aarch64-elf-gcc                 # macOS Homebrew
        aarch64-linux-gnu-gcc           # Linux (Ubuntu/Debian)
        aarch64-none-linux-gnu-gcc      # ARM official / Windows
        aarch64-unknown-linux-gnu-gcc   # Alternative macOS
)

if(NOT AARCH64_GCC)
    message(FATAL_ERROR 
        "ARM64 cross-compiler not found!\n"
        "Install instructions:\n"
        "  Linux:   sudo apt install g++-aarch64-linux-gnu\n"
        "  macOS:   brew install aarch64-elf-gcc\n"
        "  Windows: Download from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads"
    )
endif()

# Extract toolchain prefix from the found compiler
get_filename_component(AARCH64_GCC_NAME ${AARCH64_GCC} NAME_WE)
string(REGEX REPLACE "-gcc$" "" TOOLCHAIN_PREFIX ${AARCH64_GCC_NAME})

message(STATUS "Found ARM64 toolchain: ${AARCH64_GCC}")
message(STATUS "Using toolchain prefix: ${TOOLCHAIN_PREFIX}")

# Set compilers using the detected prefix
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_AR ${TOOLCHAIN_PREFIX}-ar)
set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}-ld)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}-objcopy)

# Disable compiler checks
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_ASM_COMPILER_WORKS TRUE)

# Bare-metal flags
set(CMAKE_C_FLAGS "-ffreestanding -nostdlib -O2 -Wall -Wextra" CACHE STRING "C compiler flags")
set(CMAKE_ASM_FLAGS "" CACHE STRING "ASM compiler flags")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib" CACHE STRING "Linker flags")
