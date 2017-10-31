cmake_minimum_required(VERSION 2.8.8)
project(IJST)

set(CMAKE_CXX_STANDARD 11)

#TODO: inlucde gTest and rapidjson as third party
include_directories(include)

add_subdirectory(unit_test)