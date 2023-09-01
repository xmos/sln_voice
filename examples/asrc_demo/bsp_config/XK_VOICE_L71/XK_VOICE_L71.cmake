
## Create custom board targets for application
set(TARGET_NAME sln_voice_app_asrc_demo_board_support_xk_voice_l71)
add_library(${TARGET_NAME} INTERFACE)
target_sources(${TARGET_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/platform/oe_enable.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/driver_instances.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/platform_init.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/platform_start.c
)
target_include_directories(${TARGET_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(${TARGET_NAME}
    INTERFACE
        core::general
        rtos::freertos
        rtos::drivers::general
        rtos::drivers::usb
        rtos::drivers::dfu_image
)
target_compile_options(${TARGET_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
)
target_link_options(${TARGET_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
)
target_compile_definitions(${TARGET_NAME}
    INTERFACE
        XK_VOICE_L71=1
        PLATFORM_SUPPORTS_TILE_0=1
        PLATFORM_SUPPORTS_TILE_1=1
        PLATFORM_SUPPORTS_TILE_2=0
        PLATFORM_SUPPORTS_TILE_3=0
        USB_TILE_NO=0
        USB_TILE=tile[USB_TILE_NO]
        MIC_ARRAY_CONFIG_PDM_FREQ=3072000
        MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME=240
        MIC_ARRAY_CONFIG_MIC_COUNT=2
        MIC_ARRAY_CONFIG_CLOCK_BLOCK_A=XS1_CLKBLK_1
        MIC_ARRAY_CONFIG_CLOCK_BLOCK_B=XS1_CLKBLK_2
        MIC_ARRAY_CONFIG_PORT_MCLK=PORT_MCLK_IN_OUT
        MIC_ARRAY_CONFIG_PORT_PDM_CLK=PORT_PDM_CLK
        MIC_ARRAY_CONFIG_PORT_PDM_DATA=PORT_PDM_DATA
        MIC_ARRAY_SAMPLING_FREQ=48000
        appconfAUDIO_PIPELINE_CHANNELS=2
)

## Create an alias
add_library(sln_voice::app::asrc_demo::xk_voice_l71 ALIAS ${TARGET_NAME})
