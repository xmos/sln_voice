
#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
     ${CMAKE_CURRENT_LIST_DIR}/src/wav
)

include(${CMAKE_CURRENT_LIST_DIR}/bsp_config/bsp_config.cmake)

#if(FFVA_PIPELINES)
    set(AUDIO_PIPELINE_LIBRARY sln_voice::app::ffva::ap::adec)
    set(AUDIO_PIPELINE_INPUT_CHANNELS 4)
#endif()

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
    XSCOPE_HOST_IO_ENABLED=1
    XSCOPE_HOST_IO_TILE=0
    appconfINPUT_CHANNELS=${AUDIO_PIPELINE_INPUT_CHANNELS}
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_COMMON_LINK_LIBRARIES
    rtos::freertos
    xscope_fileio
    sln_voice_test_pipeline_board_support_xk_voice_l71
)

set(FFVA_AP adec)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_test_pipeline)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        ${APP_COMPILE_DEFINITIONS}
        THIS_XCORE_TILE=0
)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME}
    PUBLIC
        ${APP_COMMON_LINK_LIBRARIES}
        ${AUDIO_PIPELINE_LIBRARY}
)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_test_pipeline)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        ${APP_COMPILE_DEFINITIONS}
        THIS_XCORE_TILE=1
)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME}
    PUBLIC
        ${APP_COMMON_LINK_LIBRARIES}
        ${AUDIO_PIPELINE_LIBRARY}
)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(test_pipeline tile0_test_pipeline tile1_test_pipeline 1)

#**********************
# Create run and debug targets
#**********************
add_custom_target(run_test_pipeline
  COMMAND xrun --xscope-realtime --xscope-port localhost:12345 test_pipeline.xe
  DEPENDS test_pipeline
  COMMENT
    "Run application"
  VERBATIM
)

create_debug_target(test_pipeline)
