#**********************
# Gather Sources
#**********************
file(GLOB APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c ${CMAKE_CURRENT_LIST_DIR}/src/usb/*.c ${CMAKE_CURRENT_LIST_DIR}/src/gpio_test/*.c ${CMAKE_CURRENT_LIST_DIR}/src/shared/*.c)
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/usb
    ${CMAKE_CURRENT_LIST_DIR}/src/shared
)

include(${CMAKE_CURRENT_LIST_DIR}/bsp_config/bsp_config.cmake)

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
    rtos::freertos_usb
    rtos::drivers::custom_i2s_with_rate_calc
    lib_src
)

#**********************
# XMOS Example Design Targets
#**********************
include(${CMAKE_CURRENT_LIST_DIR}/src/i2s_driver/i2s_driver.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/asrc_demo_ua.cmake)
