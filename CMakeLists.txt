cmake_minimum_required(VERSION 4.1.2)
project(bootloader)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(filesystem/vio)
add_subdirectory(filesystem/fat)

# Change main.c to whatever actual bootloader program is going to be run
add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PUBLIC fat)
target_link_libraries(${PROJECT_NAME} PUBLIC vio)
