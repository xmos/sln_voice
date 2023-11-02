
set(ASRC_DEMO_COMPILE_DEFINITIONS
    ${APP_COMPILE_DEFINITIONS}
    appconfI2S_ENABLED=1
    appconfUSB_ENABLED=1
    appconfI2S_MODE=appconfI2S_MODE_SLAVE
    appconfUSB_AUDIO_SAMPLE_RATE=48000
)


#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_asrc_demo)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        ${ASRC_DEMO_COMPILE_DEFINITIONS}
        THIS_XCORE_TILE=0
)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME}
    PUBLIC
        ${APP_COMMON_LINK_LIBRARIES}
        sln_voice::app::asrc_demo::xk_voice_l71
)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_asrc_demo)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        ${ASRC_DEMO_COMPILE_DEFINITIONS}
        THIS_XCORE_TILE=1
)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME}
    PUBLIC
        ${APP_COMMON_LINK_LIBRARIES}
        sln_voice::app::asrc_demo::xk_voice_l71
)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_asrc_demo tile0_example_asrc_demo tile1_example_asrc_demo 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_asrc_demo)
create_debug_target(example_asrc_demo)
create_upgrade_img_target(example_asrc_demo ${XTC_VERSION_MAJOR} ${XTC_VERSION_MINOR})

#**********************
# Create data partition support targets
#**********************
set(TARGET_NAME example_asrc_demo)

create_flash_app_target(
    #[[ Target ]]                  ${TARGET_NAME}
)
