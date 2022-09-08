## **********************************
## Fetch and patch lib_tflite_micro
## **********************************
include(FetchContent)

message(STATUS "Fetching dependency lib_nn")
FetchContent_Declare(
    lib_nn
    GIT_REPOSITORY https://github.com/xmos/lib_nn.git
    GIT_TAG        f85b4804ea5f52f5fa4ca4b709a787ac62a8c526
    GIT_SHALLOW    TRUE
    SOURCE_DIR     ${CMAKE_BINARY_DIR}/dependencies/lib_nn
)
FetchContent_Populate(lib_nn)

message(STATUS "Fetching dependency lib_tflite_micro")
FetchContent_Declare(
    lib_tflite_micro
    GIT_REPOSITORY https://github.com/xmos/lib_tflite_micro.git
    GIT_TAG        a2dccc68bdef57a3b0ed326ea4d815a475feb39e
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES_RECURSE TRUE
    SOURCE_DIR     ${CMAKE_BINARY_DIR}/dependencies/lib_tflite_micro
    PATCH_COMMAND  git apply --directory lib_tflite_micro/submodules/tflite-micro/tensorflow ${CMAKE_BINARY_DIR}/dependencies/lib_tflite_micro/patches/tflite-micro.patch
)
FetchContent_Populate(lib_tflite_micro)

SET(TOP_DIR ${CMAKE_BINARY_DIR}/dependencies/lib_tflite_micro)
include(${TOP_DIR}/cmakefiles/xtflm.cmake)

## ********************
## Create lib_nn target
## ********************
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    add_library(inferencing_lib_nn STATIC)

    ## Source files
    file(GLOB_RECURSE NN_SOURCES_ASM  ${TOP_DIR}/lib_nn/lib_nn/src/asm/*.S)

    ## cmake doesn't recognize .S files as assembly by default
    set_source_files_properties(LIB_NN_SOURCES_ASM PROPERTIES LANGUAGE ASM)

    ## Assume all asm is XS3A for now
    set(XCORE_XS3A_SOURCES ${NN_SOURCES_ASM})

    target_sources(inferencing_lib_nn
        PRIVATE
            ${NN_SOURCES}
            ${NN_SOURCES_ASM}
    )
    target_include_directories(inferencing_lib_nn
        PUBLIC
            ${NNLIB_DIR}/api
            ${NNLIB_DIR}/..
    )
    target_compile_options(inferencing_lib_nn
        PRIVATE
            "-Os"
            "-Wno-xcore-fptrgroup"
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## ******************************
    ## Create lib_tflite_micro target
    ## ******************************
    add_library(inferencing_tflite_micro STATIC)

    ## Source files
    file(GLOB_RECURSE TFLIB_SOURCES_ASM ${TOP_DIR}/lib_tflite_micro/src/*.S)
    set_source_files_properties(TFLIB_SOURCES_ASM PROPERTIES LANGUAGE ASM)

    target_sources(inferencing_tflite_micro
        PRIVATE
            ${TFLIB_SOURCES_ASM}
            ${TFLITE_SOURCES}
            ${TFLM_KERNEL_SOURCES}
            ${XTFLIB_KERNEL_SOURCES}
    )
    target_include_directories(inferencing_tflite_micro
        PRIVATE
            "src"
            "${TFLIB_DIR}/.."
            "${TFLIB_DIR}/api"
            "${TFLIB_DIR}/submodules/tflite-micro"
            "${TFLIB_DIR}/submodules/gemmlowp"
            "${TFLIB_DIR}/submodules/ruy"
            "${TFLIB_DIR}/submodules/flatbuffers/include"
            "${TFLIB_DIR}/src/tflite-xcore-kernels"
    )
    target_link_libraries(inferencing_tflite_micro
        PRIVATE
            inferencing_lib_nn
    )
    target_compile_options(inferencing_tflite_micro
        PRIVATE
            "-Os"
            "-Wno-xcore-fptrgroup"
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )
    target_compile_definitions(inferencing_tflite_micro
        PRIVATE
            NO_INTERPRETER
            TF_LITE_STATIC_MEMORY
            TF_LITE_STRIP_ERROR_STRINGS
            __xtflm_conf_h_exists__
    )

    add_library(sln_voice::inferencing::lib_tflite_micro ALIAS inferencing_tflite_micro)
endif()
