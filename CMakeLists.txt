cmake_minimum_required(VERSION 3.19)
project(scheduler)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.c*")
add_executable(${PROJECT_NAME} ${SOURCES} main.cpp)
