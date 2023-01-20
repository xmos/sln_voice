#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/usb
    ${CMAKE_CURRENT_LIST_DIR}/src/ww_model_runner
)

include(${CMAKE_CURRENT_LIST_DIR}/bsp_config/bsp_config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/audio_pipeline/audio_pipeline.cmake)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -g
    -report
    -fxscope
    -mcmodel=large
    -Wno-xcore-fptrgroup
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    XUD_CORE_CLOCK=600

    CFG_TUSB_DEBUG_PRINTF=rtos_printf
    CFG_TUSB_DEBUG=0
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_COMMON_LINK_LIBRARIES
    inferencing_tflite_micro
    rtos::freertos_usb
    lib_src
)

#**********************
# Pipeline Options
# By default only these targets are created:
#  example_stlp_int_fixed_delay
#  example_stlp_ua_adec
#**********************
option(ENABLE_ALL_STLP_PIPELINES  "Create all STLP pipeline configurations"  OFF)

if(ENABLE_ALL_STLP_PIPELINES)
    set(STLP_PIPELINES_INT
        fixed_delay
        adec
        adec_altarch
    )

    set(STLP_PIPELINES_UA
        fixed_delay
        adec
        adec_altarch
    )
else()
    set(STLP_PIPELINES_INT
        fixed_delay
    )

    set(STLP_PIPELINES_UA
        adec
    )
endif()

#**********************
# XMOS Example Design Targets
#**********************
include(${CMAKE_CURRENT_LIST_DIR}/stlp_int.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/stlp_ua.cmake)
