cmake_minimum_required(VERSION 3.20)
set (CMAKE_CXX_STANDARD 20)

PROJECT(Find)

add_subdirectory(fmt)
include_directories(${CMAKE_SOURCE_DIR}/fmt/include)
if (WIN32)
    add_compile_definitions(WINDOWS=on)
endif(WIN32)

add_executable(Find src/Main.cpp src/find.cpp)
target_link_libraries(Find PRIVATE fmt::fmt)
