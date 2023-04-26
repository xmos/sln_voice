set(TARGET_FILE ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn)
set(XSCOPE_PORT localhost:12345)

include(${CMAKE_CURRENT_LIST_DIR}/asr_example/asr_example.cmake)

#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/src/*.xc 
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
)
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/src
)

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
    ${TARGET_FILE}
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
)

set(APP_LINK_OPTIONS
    -report
    ${TARGET_FILE}
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_LINK_LIBRARIES
    -lquadspi
    sln_voice::app::asr::example
    lib_xcore_math
    xscope_fileio
)

#**********************
#  Targets
#**********************
add_executable(example_asr)
target_sources(example_asr PUBLIC ${APP_SOURCES})
target_include_directories(example_asr PUBLIC ${APP_INCLUDES})
target_compile_definitions(example_asr PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(example_asr PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(example_asr PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(example_asr PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create run and debug targets
#**********************
add_custom_target(run_example_asr
  COMMAND xrun --xscope-realtime --xscope-port ${XSCOPE_PORT} example_asr.xe
  DEPENDS example_asr
  COMMENT
    "Run application"
  VERBATIM
)
