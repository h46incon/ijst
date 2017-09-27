cmake_minimum_required(VERSION 3.7)
project(Test1)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

set(SOURCE_FILES
        main.cpp
        )
add_executable(Test1 ${SOURCE_FILES})