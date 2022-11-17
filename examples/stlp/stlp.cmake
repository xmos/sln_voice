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
    core::lib_tflite_micro
    rtos::freertos_usb
    sdk::lib_src
)

set(STLP_PIPELINES
    fixed_delay
    adec
    adec_altarch
)

#**********************
# XMOS Example Design Targets
#**********************
include(${CMAKE_CURRENT_LIST_DIR}/stlp_int.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/stlp_ua.cmake)

#**********************
# XMOS Development Targets
#**********************
include(${CMAKE_CURRENT_LIST_DIR}/stlp_dev.cmake)
