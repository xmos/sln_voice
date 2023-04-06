
## Create custom board targets for application
add_library(sln_voice_test_pipeline_board_support_xk_voice_l71 INTERFACE)
target_sources(sln_voice_test_pipeline_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/platform/driver_instances.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/platform_init.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/platform_start.c
)
target_include_directories(sln_voice_test_pipeline_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(sln_voice_test_pipeline_board_support_xk_voice_l71
    INTERFACE
        core::general
        rtos::drivers::general
        rtos::freertos
)
target_compile_options(sln_voice_test_pipeline_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
)
target_link_options(sln_voice_test_pipeline_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
)
target_compile_definitions(sln_voice_test_pipeline_board_support_xk_voice_l71
    INTERFACE
        XK_VOICE_L71=1
        PLATFORM_SUPPORTS_TILE_0=1
        PLATFORM_SUPPORTS_TILE_1=1
        PLATFORM_SUPPORTS_TILE_2=0
        PLATFORM_SUPPORTS_TILE_3=0
)
