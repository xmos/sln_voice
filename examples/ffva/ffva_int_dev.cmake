
set(FFVA_INT_COMPILE_DEFINITIONS
${APP_COMPILE_DEFINITIONS}
    appconfEXTERNAL_MCLK=1
    appconfI2S_ENABLED=1
    appconfUSB_ENABLED=0
    appconfAEC_REF_DEFAULT=appconfAEC_REF_I2S
    appconfI2S_MODE=appconfI2S_MODE_SLAVE
    appconfI2S_AUDIO_SAMPLE_RATE=480000

    ## VK Voice uses 12288000 for RPI integration, EXPLORER Board uses default 24576000
    # MIC_ARRAY_CONFIG_MCLK_FREQ=12288000
)

foreach(FFVA_AP ${FFVA_PIPELINES_INT})
#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_ffva_int_dev_${FFVA_AP})
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        ${FFVA_INT_COMPILE_DEFINITIONS}
        THIS_XCORE_TILE=0
)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME}
    PUBLIC
        ${APP_COMMON_LINK_LIBRARIES}
        sln_voice::app::ffva::xcore_ai_explorer
        sln_voice::app::ffva::ap::${FFVA_AP}
)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffva_int_dev_${FFVA_AP})
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        ${FFVA_INT_COMPILE_DEFINITIONS}
        THIS_XCORE_TILE=1
)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME}
    PUBLIC
        ${APP_COMMON_LINK_LIBRARIES}
        sln_voice::app::ffva::xcore_ai_explorer
        sln_voice::app::ffva::ap::${FFVA_AP}
)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_ffva_int_dev_${FFVA_AP} tile0_example_ffva_int_dev_${FFVA_AP} tile1_example_ffva_int_dev_${FFVA_AP} 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_ffva_int_dev_${FFVA_AP})
create_debug_target(example_ffva_int_dev_${FFVA_AP})

#**********************
# Filesystem support targets
#**********************

set(FATFS_CONTENTS_DIR ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/fatmktmp)
set(FATFS_FILE ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_ffva_int_dev_${FFVA_AP}_fat.fs)
add_custom_target(
    example_ffva_int_dev_${FFVA_AP}_fat.fs ALL
    COMMAND ${CMAKE_COMMAND} -E copy demo.txt ${FATFS_CONTENTS_DIR}/fs/demo.txt
    COMMAND fatfs_mkimage --input=${FATFS_CONTENTS_DIR} --output=${FATFS_FILE}
    COMMENT
        "Create filesystem"
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
    VERBATIM
)

set_target_properties(example_ffva_int_dev_${FFVA_AP}_fat.fs PROPERTIES
    ADDITIONAL_CLEAN_FILES "${FATFS_CONTENTS_DIR};${FATFS_FILE}"
)

create_filesystem_target(example_ffva_int_dev_${FFVA_AP})
create_flash_app_target(
    #[[ Target ]]                  example_ffva_int_dev_${FFVA_AP}
    #[[ Boot Partition Size ]]     0x100000
    #[[ Data Partition Contents ]] ${FATFS_FILE}
    #[[ Dependencies ]]            make_fs_example_ffva_int_dev_${FFVA_AP}
)
endforeach()
