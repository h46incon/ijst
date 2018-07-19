cmake_minimum_required(VERSION 2.8.8)
project(ijst)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()

SET(CMAKE_VERBOSE_MAKEFILE ON)
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    add_definitions(/wd4996)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    if(MSVC11)
        # required for VS2012 due to missing support for variadic templates
        add_definitions(-D_VARIADIC_MAX=10)
    endif(MSVC11)
else()
    add_definitions(-Wall)
    add_definitions(-Wunused-parameter)
endif()
#add_definitions(-ftime-report)

if (WIN32 AND (NOT CYGWIN) AND (NOT MINGW))
    set(gtest_disable_pthreads ON)
    set(gtest_force_shared_crt ON)
endif()
add_subdirectory(thirdparty/googletest ${CMAKE_BINARY_DIR}/gtest)

include_directories(
        include
        thirdparty/rapidjson/include
)

add_subdirectory(unit_test)