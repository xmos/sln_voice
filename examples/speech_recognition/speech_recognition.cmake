set(TARGET_FILE ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn)
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
    sln_voice::app::asr::example
    lib_xcore_math
    lib_qspi_fast_read
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
# Create data partition support targets
#**********************
set(FLASH_CAL_FILE ${LIB_QSPI_FAST_READ_ROOT_PATH}/lib_qspi_fast_read/calibration_pattern_nibble_swap.bin)
set(MODEL_FILE ${CMAKE_CURRENT_LIST_DIR}/asr_example/asr_example_model.dat)
set(DATA_PARTITION_FILE example_asr_data_partition.bin)

set(CALIBRATION_PATTERN_DATA_PARTITION_OFFSET 0)
math(EXPR MODEL_DATA_PARTITION_OFFSET
    "${LIB_QSPI_FAST_READ_DEFAULT_CAL_SIZE_BYTES}"
    OUTPUT_FORMAT HEXADECIMAL
)

add_custom_command(
    OUTPUT ${DATA_PARTITION_FILE}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${DATA_PARTITION_FILE}
    COMMAND datapartition_mkimage -v -b 1
    -i ${FLASH_CAL_FILE}:${CALIBRATION_PATTERN_DATA_PARTITION_OFFSET} ${MODEL_FILE}:${MODEL_DATA_PARTITION_OFFSET}
    -o ${DATA_PARTITION_FILE}
    DEPENDS
        ${MODEL_FILE}
        ${FLASH_CAL_FILE}
    COMMENT
        "Create data partition"
    VERBATIM
)

create_data_partition_directory(
    #[[ Target ]]                   example_asr
    #[[ Copy Files ]]               "${DATA_PARTITION_FILE}"
    #[[ Dependencies ]]             "${DATA_PARTITION_FILE}"
)

add_custom_target(flash_app_example_asr
    COMMAND xflash --write-all ${DATA_PARTITION_FILE} --target-file=${TARGET_FILE}
    COMMENT
        "Flash data partition"
)

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
