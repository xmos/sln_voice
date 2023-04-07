
#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/wav
)

include(${CMAKE_CURRENT_LIST_DIR}/bsp_config/bsp_config.cmake)

#**********************
# Setup pipeline test to build
#**********************
set(TEST_PIPELINE "FFVA_ADEC;FFD" CACHE STRING
  "List of possible values for the TEST_PIPELINE variable")

if(${TEST_PIPELINE} STREQUAL "FFVA_ADEC")
    message(STATUS "Building FFVA ADEC pipeline test")
    set(AUDIO_PIPELINE_LIBRARY sln_voice::app::ffva::ap::adec)
    set(AUDIO_PIPELINE_INPUT_CHANNELS 4)
    set(TEST_PIPELINE_NAME test_pipeline_ffva_adec)
elseif(${TEST_PIPELINE} STREQUAL "FFD")
    message(STATUS "Building FFD pipeline test")
    # The FFD pipeline needs other include paths set.  Gross!
    set(APP_INCLUDES
        ${APP_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/ffd/src
    )
    set(AUDIO_PIPELINE_LIBRARY sln_voice::app::ffd::ap)
    set(AUDIO_PIPELINE_INPUT_CHANNELS 2)
    set(TEST_PIPELINE_NAME test_pipeline_ffd)
else()
    message(FATAL_ERROR "Unable to build ${TEST_PIPELINE} pipeline test")
endif()

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

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_${TEST_PIPELINE_NAME})
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

set(TARGET_NAME tile1_${TEST_PIPELINE_NAME})
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
merge_binaries(${TEST_PIPELINE_NAME} tile0_${TEST_PIPELINE_NAME} tile1_${TEST_PIPELINE_NAME} 1)

#**********************
# Create run and debug targets
#**********************
add_custom_target(run_${TEST_PIPELINE_NAME}
  COMMAND xrun --xscope-realtime --xscope-port localhost:12345 ${TEST_PIPELINE_NAME}.xe
  DEPENDS ${TEST_PIPELINE_NAME}
  COMMENT
    "Run application"
  VERBATIM
)

create_debug_target(${TEST_PIPELINE_NAME})
