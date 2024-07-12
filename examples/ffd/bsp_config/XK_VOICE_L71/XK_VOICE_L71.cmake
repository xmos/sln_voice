
## Create custom board targets for application
add_library(sln_voice_app_ffd_board_support_xk_voice_l71 INTERFACE)
target_sources(sln_voice_app_ffd_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/platform/app_pll_ctrl.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/driver_instances.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/platform_init.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/platform_start.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/dac_port.c
)
target_include_directories(sln_voice_app_ffd_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(sln_voice_app_ffd_board_support_xk_voice_l71
    INTERFACE
        core::general
        rtos::freertos
        rtos::drivers::general
        rtos::drivers::audio
        sln_voice::app::ffd::dac::dac3101
)
target_compile_options(sln_voice_app_ffd_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
)
target_link_options(sln_voice_app_ffd_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
)
target_compile_definitions(sln_voice_app_ffd_board_support_xk_voice_l71
    INTERFACE
        XK_VOICE_L71=1
        PLATFORM_SUPPORTS_TILE_0=1
        PLATFORM_SUPPORTS_TILE_1=1
        PLATFORM_SUPPORTS_TILE_2=0
        PLATFORM_SUPPORTS_TILE_3=0

        MIC_ARRAY_CONFIG_MCLK_FREQ=24576000
        MIC_ARRAY_CONFIG_PDM_FREQ=3072000
        MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME=240
        MIC_ARRAY_CONFIG_MIC_COUNT=2
        MIC_ARRAY_CONFIG_CLOCK_BLOCK_A=XS1_CLKBLK_1
        MIC_ARRAY_CONFIG_CLOCK_BLOCK_B=XS1_CLKBLK_2
        MIC_ARRAY_CONFIG_PORT_MCLK=PORT_MCLK_IN_OUT
        MIC_ARRAY_CONFIG_PORT_PDM_CLK=PORT_PDM_CLK
        MIC_ARRAY_CONFIG_PORT_PDM_DATA=PORT_PDM_DATA
)

## Create an alias
add_library(sln_voice::app::ffd::xk_voice_l71 ALIAS sln_voice_app_ffd_board_support_xk_voice_l71)
