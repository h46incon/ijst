cmake_minimum_required(VERSION 2.8.8)
project(IJST)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-Wall)
add_definitions(-Wunused-parameter)
#add_definitions(-DIJST_USE_SL_WRAPPER)
#add_definitions(-DIJST_ENABLE_TO_JSON_OBJECT)
#add_definitions(-DIJST_ENABLE_FROM_JSON_OBJECT)
#add_definitions(-ftime-report)

include_directories(
        include
        thirdparty/rapidjson/include
)

add_subdirectory(thirdparty/googletest)
add_subdirectory(unit_test)