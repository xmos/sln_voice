#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
list(APPEND APP_SOURCES ${SOLUTION_VOICE_ROOT_PATH}/examples/low_power_ffd/src/power/low_power_audio_buffer.c)
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/src/stubs/
    ${SOLUTION_VOICE_ROOT_PATH}/examples/low_power_ffd/src/power/
)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -g
    -report
    -fxscope
    ${SOLUTION_VOICE_ROOT_PATH}/examples/low_power_ffd/bsp_config/XK_VOICE_L71/XK_VOICE_L71.xn
)

set(APP_LINK_OPTIONS
    -report
    ${SOLUTION_VOICE_ROOT_PATH}/examples/low_power_ffd/bsp_config/XK_VOICE_L71/XK_VOICE_L71.xn
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME test_ffd_low_power_audio_buffer)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
