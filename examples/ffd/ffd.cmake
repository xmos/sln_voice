#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/audio_pipeline
    ${CMAKE_CURRENT_LIST_DIR}/src/gpio_ctrl
    ${CMAKE_CURRENT_LIST_DIR}/src/intent_handler
    ${CMAKE_CURRENT_LIST_DIR}/src/intent_handler/audio_response
    ${CMAKE_CURRENT_LIST_DIR}/src/power
)
set(RTOS_CONF_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos_conf
)

include(${CMAKE_CURRENT_LIST_DIR}/bsp_config/bsp_config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/inference/inference.cmake)

#**********************
# QSPI Flash Layout
#**********************
set(BOOT_PARTITION_SIZE 0x100000)
set(FILESYSTEM_SIZE_KB 1024)
math(EXPR FILESYSTEM_SIZE_BYTES
     "1024 * ${FILESYSTEM_SIZE_KB}"
     OUTPUT_FORMAT HEXADECIMAL
)

set(FILESYSTEM_START_ADDRESS ${BOOT_PARTITION_SIZE})
math(EXPR MODEL_START_ADDRESS
     "${FILESYSTEM_START_ADDRESS} + ${FILESYSTEM_SIZE_BYTES}"
     OUTPUT_FORMAT HEXADECIMAL)

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
    configENABLE_DEBUG_PRINTF=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    QSPI_FLASH_FILESYSTEM_START_ADDRESS=${FILESYSTEM_START_ADDRESS}
    QSPI_FLASH_MODEL_START_ADDRESS=${MODEL_START_ADDRESS}
)

set(APP_LINK_OPTIONS
    -report
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_COMMON_LINK_LIBRARIES
    sln_voice::app::ffd::inference_engine::wanson
    fwk_voice::agc
    fwk_voice::ic
    fwk_voice::ns
    fwk_voice::vnr::features
    fwk_voice::vnr::inference
    rtos::drivers::clock_control
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_ffd)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${RTOS_CONF_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffd)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${RTOS_CONF_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_ffd tile0_example_ffd tile1_example_ffd 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_ffd)
create_debug_target(example_ffd)

#**********************
# Create data partition support targets
#**********************
set(TARGET_NAME example_ffd)
set(DATA_PARTITION_FILE ${TARGET_NAME}_data_partition.bin)
set(MODEL_FILE ${TARGET_NAME}_model.bin)
set(FATFS_FILE ${TARGET_NAME}_fat.fs)

add_custom_target(${MODEL_FILE} ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TARGET_NAME}_split
    COMMAND xobjdump --strip ${TARGET_NAME}.xe > ${TARGET_NAME}_split/output.log
    COMMAND xobjdump --split --split-dir ${TARGET_NAME}_split ${TARGET_NAME}.xb >> ${TARGET_NAME}_split/output.log
    COMMAND ${CMAKE_COMMAND} -E copy ${TARGET_NAME}_split/image_n0c0.swmem ${MODEL_FILE}
    DEPENDS ${TARGET_NAME}
    BYPRODUCTS
        ${TARGET_NAME}.xb
    COMMENT
        "Extract swmem"
    VERBATIM
)

set_target_properties(${MODEL_FILE} PROPERTIES
    ADDITIONAL_CLEAN_FILES "${TARGET_NAME}_split;${MODEL_FILE}"
)

create_filesystem_target(
    #[[ Target ]]                   ${TARGET_NAME}
    #[[ Input Directory ]]          ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
    #[[ Image Size ]]               ${FILESYSTEM_SIZE_BYTES}
)

add_custom_command(
    OUTPUT ${DATA_PARTITION_FILE}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${DATA_PARTITION_FILE}
    COMMAND datapartition_mkimage -v -b 1024
        -i ${FATFS_FILE}:0 ${MODEL_FILE}:${FILESYSTEM_SIZE_KB}
        -o ${DATA_PARTITION_FILE}
    DEPENDS
        ${MODEL_FILE}
        make_fs_${TARGET_NAME}
    COMMENT
        "Create data partition"
    VERBATIM
)

list(APPEND DATA_PARTITION_FILE_LIST
    ${DATA_PARTITION_FILE}
    ${MODEL_FILE}
    ${FATFS_FILE}
)

list(APPEND DATA_PARTITION_DEPENDS_LIST
    ${DATA_PARTITION_FILE}
    ${MODEL_FILE}
    make_fs_${TARGET_NAME}
)

# The list of files to copy and the dependency list for populating
# the data partition folder are identical.
create_data_partition_directory(
    #[[ Target ]]                   ${TARGET_NAME}
    #[[ Copy Files ]]               "${DATA_PARTITION_FILE_LIST}"
    #[[ Dependencies ]]             "${DATA_PARTITION_DEPENDS_LIST}"
)

create_flash_app_target(
    #[[ Target ]]                   ${TARGET_NAME}
    #[[ Boot Partition Size ]]      ${BOOT_PARTITION_SIZE}
    #[[ Data Partition Contents ]]  ${DATA_PARTITION_FILE}
    #[[ Dependencies ]]             ${DATA_PARTITION_FILE}
)

unset(DATA_PARTITION_FILE_LIST)

#**********************
# Include FFD Debug and Extension targets
#**********************
include(${CMAKE_CURRENT_LIST_DIR}/ext/ffd_ext.cmake)
