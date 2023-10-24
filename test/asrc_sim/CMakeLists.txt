cmake_minimum_required(VERSION 3.0)
project(usb_in_i2s_out)

set (CMAKE_CXX_STANDARD 17 CACHE STRING CXX_Standard)

set(NUMCPP_NO_USE_BOOST 1)
add_compile_definitions(NUMCPP_NO_USE_BOOST=1)
include(FetchContent)
FetchContent_Declare(SystemC
        GIT_REPOSITORY https://github.com/accellera-official/systemc
        GIT_TAG 2.3.4)
FetchContent_MakeAvailable(SystemC)

FetchContent_Declare(NumCpp
        GIT_REPOSITORY https://github.com/dpilger26/NumCpp
        GIT_TAG Version_2.10.1)
FetchContent_MakeAvailable(NumCpp)

FetchContent_Declare(
    lib_src
    GIT_REPOSITORY https://github.com/xmos/lib_src.git
    GIT_TAG        b5b90b0d41dcfc563ad6e879ff1c1592dd0d8034
    GIT_SHALLOW    FALSE
    SOURCE_DIR     ${CMAKE_BINARY_DIR}/_deps/lib_src
)
FetchContent_Populate(lib_src)

include(${CMAKE_BINARY_DIR}/_deps/lib_src/tests/asrc_test/asrc_c_emulator.cmake)

set(ASRC_EXAMPLE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../examples/asrc_demo/src)

## usb_in_i2s_out
add_executable(usb_in_i2s_out
    src/app_usb_in_i2s_out/main.cpp
    src/app_usb_in_i2s_out/usb.cxx
    src/app_usb_in_i2s_out/asrc.cxx
    src/app_usb_in_i2s_out/i2s.cxx
    src/app_usb_in_i2s_out/pi_control.c
    src/common/buffer/buffer.cxx
    src/common/buffer/avg_buffer_level.c
    src/common/usb_rate_calc/usb_rate_calc.c
    src/common/helpers.cpp
    ${ASRC_EXAMPLE_PATH}/shared/div.c
)
target_include_directories(usb_in_i2s_out
    PRIVATE
        src/app_usb_in_i2s_out
        src/common/config
        src/common/usb_rate_calc
        src/common/buffer
        src/common
        ${ASRC_EXAMPLE_PATH}/shared
)

target_link_libraries(usb_in_i2s_out
    NumCpp::NumCpp
)

target_link_libraries(usb_in_i2s_out SystemC::systemc asrc_c_emulator_lib )

target_compile_definitions(usb_in_i2s_out PRIVATE XCORE_MATH_NOT_INCLUDED=1)


## i2s_in_usb_out
add_executable(i2s_in_usb_out
    src/app_i2s_in_usb_out/main.cpp
    src/app_i2s_in_usb_out/usb.cxx
    src/app_i2s_in_usb_out/asrc.cxx
    src/app_i2s_in_usb_out/i2s.cxx
    src/app_i2s_in_usb_out/pi_control.c
    src/common/buffer/buffer.cxx
    src/common/buffer/avg_buffer_level.c
    src/common/usb_rate_calc/usb_rate_calc.c
    src/common/helpers.cpp
    ${ASRC_EXAMPLE_PATH}/shared/div.c
)
target_include_directories(i2s_in_usb_out
    PRIVATE
        src/app_i2s_in_usb_out
        src/common/config
        src/common/usb_rate_calc
        src/common/buffer
        src/common
        ${ASRC_EXAMPLE_PATH}/shared
)

target_link_libraries(i2s_in_usb_out
    NumCpp::NumCpp
)

target_link_libraries(i2s_in_usb_out SystemC::systemc asrc_c_emulator_lib )

target_compile_definitions(i2s_in_usb_out PRIVATE XCORE_MATH_NOT_INCLUDED=1)
