cmake_minimum_required(VERSION 3.20)
set (CMAKE_CXX_STANDARD 20)

PROJECT(Find)

if (WIN32)
    add_compile_definitions(WINDOWS=on)
endif(WIN32)

add_executable(Find src/Main.cpp src/find.cpp)

