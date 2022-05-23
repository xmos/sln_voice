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
option(SOLUTION_AVONA_CI_TESTING     "Enable CI test applications"  OFF)

## TODO these should be moved outside of the xcore_sdk repository
## Import some helpful macros
include(xcore_sdk/tools/cmake_utils/xmos_macros.cmake)

## Add frameworks
add_subdirectory(xcore_sdk)

## Add top level project targets
if(PROJECT_IS_TOP_LEVEL)
    include(applications/applications.cmake)
    include(examples/examples.cmake)
endif()

if(SOLUTION_AVONA_CI_TESTING)
    include(test/tests.cmake)
endif()
