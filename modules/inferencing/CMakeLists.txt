## ********************
## Create lib_nn target
## ********************
add_library(inferencing_lib_nn STATIC)

    ## Source files
file(GLOB_RECURSE NN_SOURCES "lib_nn/lib_nn/src/c/*.c")

list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/AggregateFn.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/AggregateFn_DW.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/Filter2D.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/MemCpyFn.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/OutputTransformFn.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/filt2d/conv2d_utils.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/filt2d/util.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/filt2d/geom/Filter2dGeometry.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/filt2d/geom/ImageGeometry.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/filt2d/geom/WindowGeometry.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/cpp/filt2d/geom/WindowLocation.cpp")
list(APPEND NN_SOURCES  "lib_nn/lib_nn/src/asm/asm_constants.c")

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)

    file(GLOB_RECURSE NN_SOURCES_ASM  lib_nn/lib_nn/src/asm/*.S)

    ## cmake doesn't recognize .S files as assembly by default
    set_source_files_properties(LIB_NN_SOURCES_ASM PROPERTIES LANGUAGE ASM)

    ## Assume all asm is XS3A for now
    set(XCORE_XS3A_SOURCES ${NN_SOURCES_ASM})

    target_compile_options(inferencing_lib_nn
        PRIVATE
            "-Os"
            "-Wno-xcore-fptrgroup"
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )
else()
    set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "C++ Compiler Base Flags" FORCE)
    target_compile_definitions(inferencing_lib_nn
        PRIVATE
            NN_USE_REF
    )    

    target_compile_options(inferencing_lib_nn
        PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )
endif()

target_sources(inferencing_lib_nn
    PRIVATE
        ${NN_SOURCES}
        ${NN_SOURCES_ASM}
)
target_include_directories(inferencing_lib_nn
    PUBLIC
        lib_nn/lib_nn/api
        lib_nn/lib_nn/..
)

## **********************************
## Fetch and patch lib_tflite_micro
## **********************************
include(FetchContent)

IF(EXISTS ${CMAKE_BINARY_DIR}/dependencies/lib_tflite_micro)
    message(STATUS "Skipped: dependencies/lib_tflite_micro")
else()
    message(STATUS "Fetching: dependencies/lib_tflite_micro")
    FetchContent_Declare(
        lib_tflite_micro
        GIT_REPOSITORY https://github.com/keithm-xmos/lib_tflite_micro.git
        GIT_TAG        aa28fd964bc30ea363a9f194bc41e1632fc7755b
        GIT_SHALLOW    TRUE
        GIT_SUBMODULES_RECURSE TRUE
        SOURCE_DIR     ${CMAKE_BINARY_DIR}/dependencies/lib_tflite_micro
        PATCH_COMMAND  git apply --directory lib_tflite_micro/submodules/tflite-micro/tensorflow ${CMAKE_BINARY_DIR}/dependencies/lib_tflite_micro/patches/tflite-micro.patch
    )
    FetchContent_Populate(lib_tflite_micro)
endif()


SET(TOP_DIR ${CMAKE_BINARY_DIR}/dependencies/lib_tflite_micro)
include(${TOP_DIR}/cmakefiles/xtflm.cmake)

## ******************************
## Create lib_tflite_micro target
## ******************************
add_library(inferencing_tflite_micro STATIC)

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Source files
    file(GLOB_RECURSE TFLIB_SOURCES_ASM ${TOP_DIR}/lib_tflite_micro/src/*.S)
    set_source_files_properties(TFLIB_SOURCES_ASM PROPERTIES LANGUAGE ASM)

    target_compile_options(inferencing_tflite_micro
        PRIVATE
            "-Os"
            "-Wno-xcore-fptrgroup"
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )
else()
    target_compile_definitions(inferencing_tflite_micro
        PRIVATE
            __xtflm_conf_h_exists__
    )
endif()

target_compile_definitions(inferencing_tflite_micro
    PUBLIC
        NO_INTERPRETER
        TF_LITE_STATIC_MEMORY
        TF_LITE_STRIP_ERROR_STRINGS
        XTFLM_DISABLED
)

target_sources(inferencing_tflite_micro
    PRIVATE
        ${TFLIB_SOURCES_ASM}
        ${TFLITE_SOURCES}
        ${TFLM_KERNEL_SOURCES}
        ${XTFLIB_KERNEL_SOURCES}
)

target_include_directories(inferencing_tflite_micro
    PUBLIC
        "${TFLIB_DIR}/../.."
        "${TFLIB_DIR}/api"
        "${TFLIB_DIR}/submodules/tflite-micro"
        "${TFLIB_DIR}/submodules/flatbuffers/include"
    PRIVATE
        "src"
        "${TFLIB_DIR}/.."
        "${TFLIB_DIR}/submodules/tflite-micro"
        "${TFLIB_DIR}/submodules/gemmlowp"
        "${TFLIB_DIR}/submodules/ruy"
        "${TFLIB_DIR}/src/tflite-xcore-kernels"
)

target_link_libraries(inferencing_tflite_micro
    PRIVATE
        inferencing_lib_nn
)
