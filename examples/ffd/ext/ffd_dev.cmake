#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_ffd_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${RTOS_CONF_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0 appconfSSD1306_DISPLAY_ENABLED=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffd_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${RTOS_CONF_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1 appconfSSD1306_DISPLAY_ENABLED=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_ffd_dev tile0_example_ffd_dev tile1_example_ffd_dev 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_ffd_dev)
create_debug_target(example_ffd_dev)
create_filesystem_target(example_ffd_dev)
create_flash_app_target(example_ffd_dev)

#**********************
# Create filesystem support targets
#**********************
add_custom_command(
    OUTPUT example_ffd_dev_model.bin
    COMMAND xobjdump --strip example_ffd_dev.xe
    COMMAND xobjdump --split example_ffd_dev.xb
    COMMAND ${CMAKE_COMMAND} -E copy image_n0c0.swmem ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/model.bin
    DEPENDS example_ffd_dev
    COMMENT
        "Extract swmem"
    VERBATIM
)

add_custom_command(
    OUTPUT example_ffd_dev_fat.fs
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_ffd_dev_fat.fs
    COMMAND fatfs_mkimage --input=${CMAKE_CURRENT_LIST_DIR}/filesystem_support --image_size=2097152 --output=example_ffd_dev_fat.fs
    DEPENDS example_ffd_dev_model.bin
    COMMENT
        "Create filesystem"
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
    VERBATIM
)

add_custom_target(flash_fs_example_ffd_dev
    COMMAND xflash --quad-spi-clock 50MHz --factory example_ffd_dev.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_ffd_dev_fat.fs
    DEPENDS example_ffd_dev_fat.fs
    COMMENT
        "Flash filesystem"
    VERBATIM
)