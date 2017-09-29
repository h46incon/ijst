cmake_minimum_required(VERSION 2.8.8)


add_subdirectory(gtest-1.8.0)

enable_testing()

include_directories(${gtest_SOURCE_DIR})
include_directories(${gtest_SOURCE_DIR}/include)

set(TEST_OUTPUT unit_test)
set(TEST_SRC
        serialize_test.cpp
        )
# NOTE: Please check output target here
# if target is executable:
add_executable(${TEST_OUTPUT} ${TEST_SRC})

target_link_libraries(${TEST_OUTPUT} gtest gtest_main)
