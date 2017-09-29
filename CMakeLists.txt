cmake_minimum_required(VERSION 2.8.8)
project(IJST)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

set(SOURCE_FILES
        main.cpp
        )
add_executable(IJST ${SOURCE_FILES})

add_subdirectory(unit_test)