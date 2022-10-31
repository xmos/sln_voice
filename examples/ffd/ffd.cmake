#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/ssd1306
    ${CMAKE_CURRENT_LIST_DIR}/src/intent_handler/audio_response
)
set(RTOS_CONF_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos_conf
)

include(${CMAKE_CURRENT_LIST_DIR}/bsp_config/bsp_config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/inference/inference.cmake)

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
create_filesystem_target(example_ffd)
create_flash_app_target(example_ffd)

#**********************
# Create filesystem support targets
#**********************
add_custom_command(
    OUTPUT example_ffd_model.bin
    COMMAND xobjdump --strip example_ffd.xe
    COMMAND xobjdump --split example_ffd.xb
    COMMAND ${CMAKE_COMMAND} -E copy image_n0c0.swmem ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/model.bin
    DEPENDS example_ffd
    COMMENT
        "Extract swmem"
    VERBATIM
)

add_custom_command(
    OUTPUT example_ffd_fat.fs
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_ffd_fat.fs
    COMMAND fatfs_mkimage --input=${CMAKE_CURRENT_LIST_DIR}/filesystem_support --image_size=2097152 --output=example_ffd_fat.fs
    DEPENDS example_ffd_model.bin
    COMMENT
        "Create filesystem"
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
    VERBATIM
)

add_custom_target(flash_fs_example_ffd
    COMMAND xflash --quad-spi-clock 50MHz --factory example_ffd.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_ffd_fat.fs
    DEPENDS example_ffd_fat.fs
    COMMENT
        "Flash filesystem"
    VERBATIM
)

#**********************
# Include FFD Debug and Extension targets
#**********************
include(${CMAKE_CURRENT_LIST_DIR}/ext/ffd_ext.cmake)
