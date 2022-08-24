set(KEYWORD_USB_VENDOR_OUTPUT
    appconfINFERENCE_I2C_OUTPUT_ENABLED=0
    appconfUSB_ENABLED=1
    appconfINFERENCE_USB_OUTPUT_ENABLED=1
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_ffd_usb)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_VENDOR_OUTPUT} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71_ext)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffd_usb)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_VENDOR_OUTPUT} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71_ext)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

set(TARGET_NAME tile0_example_ffd_usb_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_VENDOR_OUTPUT} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffd_usb_dev)
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
merge_binaries(example_ffd_usb tile0_example_ffd_usb tile1_example_ffd_usb 1)
merge_binaries(example_ffd_usb_dev tile0_example_ffd_usb_dev tile1_example_ffd_usb_dev 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_ffd_usb)
create_debug_target(example_ffd_usb)
create_flash_app_target(example_ffd_usb)

create_run_target(example_ffd_usb_dev)
create_debug_target(example_ffd_usb_dev)
create_flash_app_target(example_ffd_usb_dev)
