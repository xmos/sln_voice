

## Add frameworks
add_subdirectory(core)


add_subdirectory(io)
add_subdirectory(lib_qspi_fast_read)
add_subdirectory(rtos)

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Need to guard so host targets will not be built
    add_subdirectory(voice)
    add_subdirectory(sw_pll/lib_sw_pll)
endif()

## Add additional modules
add_subdirectory(asr)
add_subdirectory(audio_pipelines)
add_subdirectory(sample_rate_conversion)
add_subdirectory(xscope_fileio)
