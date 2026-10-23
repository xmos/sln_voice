

## Add frameworks
add_subdirectory(core)


add_subdirectory(io)
add_subdirectory(lib_qspi_fast_read)
add_subdirectory(rtos)

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Need to guard so host targets will not be built
    ## lib_voice::vnr links the ai_tools target, which XCommon CMake fetches as a sandbox repo.
    ## This build uses the tflite_micro runtime from the xmos-ai-tools python package instead.
    if(NOT TARGET ai_tools)
        execute_process(
            COMMAND python -c "import os; import xmos_ai_tools.runtime as rt; print(os.path.dirname(rt.__file__))"
            OUTPUT_VARIABLE XMOS_AITOOLSLIB_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(XMOS_AITOOLSLIB_PATH_CMAKE "${XMOS_AITOOLSLIB_PATH}/buildfiles/aitoolslib.cmake")
        if(XMOS_AITOOLSLIB_PATH STREQUAL "")
            message(FATAL_ERROR "Path to XMOS AI tools NOT found")
        elseif(NOT EXISTS ${XMOS_AITOOLSLIB_PATH_CMAKE})
            message(FATAL_ERROR "Cmake file 'aitoolslib.cmake' NOT found in this path")
        endif()
        message(STATUS "Found python package xmos-ai-tools at: ${XMOS_AITOOLSLIB_PATH}")
        include(${XMOS_AITOOLSLIB_PATH_CMAKE})
        add_library(ai_tools STATIC IMPORTED GLOBAL)
        target_compile_definitions(ai_tools INTERFACE ${XMOS_AITOOLSLIB_DEFINITIONS})
        set_target_properties(ai_tools PROPERTIES
            LINKER_LANGUAGE CXX
            IMPORTED_LOCATION ${XMOS_AITOOLSLIB_LIBRARIES}
            INTERFACE_INCLUDE_DIRECTORIES ${XMOS_AITOOLSLIB_INCLUDES})
    endif()
    add_subdirectory(voice/lib_voice)
    add_subdirectory(sw_pll/lib_sw_pll)
endif()

## Add additional modules
add_subdirectory(asr)
add_subdirectory(audio_pipelines)
add_subdirectory(sample_rate_conversion)
add_subdirectory(xscope_fileio)
