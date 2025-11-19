cmake_minimum_required(VERSION 4.1.2)
project(vio)
add_library(${PROJECT_NAME} STATIC vio.c)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
