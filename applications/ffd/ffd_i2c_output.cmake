set(KEYWORD_I2C_OUTPUT
    appconfINFERENCE_I2C_OUTPUT_ENABLED=1
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_application_ffd_i2c)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${KEYWORD_I2C_OUTPUT} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} sln_avona::app::ffd::xk_voice_l71)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_application_ffd_i2c)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${KEYWORD_I2C_OUTPUT} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} sln_avona::app::ffd::xk_voice_l71)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

set(TARGET_NAME tile0_application_ffd_i2c_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${KEYWORD_I2C_OUTPUT} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} sln_avona::app::ffd::xcore_ai_explorer)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_application_ffd_i2c_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${KEYWORD_I2C_OUTPUT} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} sln_avona::app::ffd::xcore_ai_explorer)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(application_ffd_i2c tile0_application_ffd_i2c tile1_application_ffd_i2c 1)
merge_binaries(application_ffd_i2c_dev tile0_application_ffd_i2c_dev tile1_application_ffd_i2c_dev 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(application_ffd_i2c)
create_debug_target(application_ffd_i2c)
create_flash_app_target(application_ffd_i2c)

create_run_target(application_ffd_i2c_dev)
create_debug_target(application_ffd_i2c_dev)
create_flash_app_target(application_ffd_i2c_dev)
