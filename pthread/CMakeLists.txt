cmake_minimum_required(VERSION 3.23)
project(OS_LAB_MATRIX_LINUX)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_executable(OS_LAB_MATRIX_LINUX main.cpp)
