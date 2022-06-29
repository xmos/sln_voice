set(KEYWORD_USB_VENDOR_OUTPUT
    appconfUSB_ENABLED=1
    appconfUSB_AUDIO_ENABLED=1
    appconfUSB_AUDIO_MODE=appconfUSB_AUDIO_TESTING
    appconfMIC_SRC_DEFAULT=appconfMIC_SRC_USB
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_application_ffd_usb_audio_test)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_VENDOR_OUTPUT} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71_ext)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_application_ffd_usb_audio_test)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_VENDOR_OUTPUT} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71_ext)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

set(TARGET_NAME tile0_application_ffd_usb_audio_test_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_VENDOR_OUTPUT} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_application_ffd_usb_audio_test_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_VENDOR_OUTPUT} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(application_ffd_usb_audio_test tile0_application_ffd_usb_audio_test tile1_application_ffd_usb_audio_test 1)
merge_binaries(application_ffd_usb_audio_test_dev tile0_application_ffd_usb_audio_test_dev tile1_application_ffd_usb_audio_test_dev 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(application_ffd_usb_audio_test)
create_debug_target(application_ffd_usb_audio_test)
create_flash_app_target(application_ffd_usb_audio_test)

create_run_target(application_ffd_usb_audio_test_dev)
create_debug_target(application_ffd_usb_audio_test_dev)
create_flash_app_target(application_ffd_usb_audio_test_dev)
