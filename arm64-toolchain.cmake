# ARM64 Cross-Compilation Toolchain for QEMU bare-metal
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Find the cross-compiler
find_program(AARCH64_GCC aarch64-linux-gnu-gcc)
find_program(AARCH64_LD aarch64-linux-gnu-ld)

if(NOT AARCH64_GCC)
    message(FATAL_ERROR "aarch64-linux-gnu-gcc not found. Install: sudo apt install gcc-aarch64-linux-gnu")
endif()

# Set compilers using full paths
set(CMAKE_C_COMPILER ${AARCH64_GCC})
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_ASM_COMPILER ${AARCH64_GCC})
set(CMAKE_AR aarch64-linux-gnu-ar)
set(CMAKE_LINKER aarch64-linux-gnu-ld)
set(CMAKE_OBJCOPY aarch64-linux-gnu-objcopy)

# Disable compiler checks
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_ASM_COMPILER_WORKS TRUE)

# Bare-metal flags
set(CMAKE_C_FLAGS "-ffreestanding -nostdlib -O2 -Wall -Wextra" CACHE STRING "C compiler flags")
set(CMAKE_ASM_FLAGS "" CACHE STRING "ASM compiler flags")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib" CACHE STRING "Linker flags")
