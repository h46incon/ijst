cmake_minimum_required(VERSION 2.8.8)

enable_testing()

include_directories(${gtest_INCLUDE_DIRS})

set(TEST_OUTPUT unit_test)
set(TEST_SRC
        basic_api_test.cpp
        serialize_test.cpp
        deserialize_test.cpp
        primitive_test.cpp
        detail_test.cpp
        extern_template_extern_test.cpp
        extern_template_explicit_test.cpp
        )

add_executable(${TEST_OUTPUT} ${TEST_SRC})

target_link_libraries(${TEST_OUTPUT} gtest gtest_main)
