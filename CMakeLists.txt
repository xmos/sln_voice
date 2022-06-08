cmake_minimum_required(VERSION 3.21)

## Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Project declaration
project(solution_avona)

## Enable languages for project
enable_language(CXX C ASM)

## Project options
option(SOLUTION_AVONA_TESTS     "Enable solution avona tests"  OFF)

## Add frameworks
add_subdirectory(xcore_sdk)

## Add top level project targets
if(PROJECT_IS_TOP_LEVEL)
    include(applications/applications.cmake)
    include(examples/examples.cmake)
endif()

if(SOLUTION_AVONA_TESTS)
    include(test/tests.cmake)
endif()
