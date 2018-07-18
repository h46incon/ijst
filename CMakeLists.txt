cmake_minimum_required(VERSION 2.8.8)
project(IJST)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
add_definitions(-Wall)
add_definitions(-Wunused-parameter)
#add_definitions(-ftime-report)

include_directories(
        include
        thirdparty/rapidjson/include
)

add_subdirectory(thirdparty/googletest)
add_subdirectory(unit_test)