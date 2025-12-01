cmake_minimum_required(VERSION 4.1.2)
project(uart)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(uart STATIC uart.c uart.h)
target_include_directories(uart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})