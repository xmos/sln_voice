set(KEYWORD_USB_TESTING
    appconfUSB_ENABLED=1
    appconfUSB_AUDIO_ENABLED=1
    appconfUSB_AUDIO_MODE=appconfUSB_AUDIO_TESTING
    appconfMIC_SRC_DEFAULT=appconfMIC_SRC_USB
    appconfINFERENCE_RAW_OUTPUT=1
    appconfAUDIO_PLAYBACK_ENABLED=0
    appconfINFERENCE_UART_OUTPUT_ENABLED=0
)

set(BYPASS_AUDIOPIPELINE_DEFINITIONS
    appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR=1
    appconfAUDIO_PIPELINE_SKIP_NS=1
    appconfAUDIO_PIPELINE_SKIP_AGC=1
    appconfLOW_POWER_ENABLED=0
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_ffd_usb_audio_test)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_TESTING} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71_ext)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffd_usb_audio_test)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_TESTING} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71_ext)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

set(TARGET_NAME tile0_example_ffd_usb_audio_test_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_TESTING} THIS_XCORE_TILE=0 appconfSSD1306_DISPLAY_ENABLED=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer_dev)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffd_usb_audio_test_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_TESTING} THIS_XCORE_TILE=1 appconfSSD1306_DISPLAY_ENABLED=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer_dev)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

set(TARGET_NAME tile0_example_ffd_usb_audio_test_bypass_ap)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_TESTING} ${BYPASS_AUDIOPIPELINE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71_ext)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffd_usb_audio_test_bypass_ap)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_TESTING} ${BYPASS_AUDIOPIPELINE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xk_voice_l71_ext)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

set(TARGET_NAME tile0_example_ffd_usb_audio_test_bypass_ap_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_TESTING} ${BYPASS_AUDIOPIPELINE_DEFINITIONS} THIS_XCORE_TILE=0 appconfSSD1306_DISPLAY_ENABLED=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer_dev)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_ffd_usb_audio_test_bypass_ap_dev)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${APP_EXT_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${APP_EXT_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${APP_EXT_COMPILE_DEFINITIONS} ${KEYWORD_USB_TESTING} ${BYPASS_AUDIOPIPELINE_DEFINITIONS} THIS_XCORE_TILE=1 appconfSSD1306_DISPLAY_ENABLED=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_EXT_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES} ${APP_EXT_COMMON_LINK_LIBRARIES} sln_voice::app::ffd::xcore_ai_explorer_dev)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_ffd_usb_audio_test tile0_example_ffd_usb_audio_test tile1_example_ffd_usb_audio_test 1)
merge_binaries(example_ffd_usb_audio_test_dev tile0_example_ffd_usb_audio_test_dev tile1_example_ffd_usb_audio_test_dev 1)

merge_binaries(example_ffd_usb_audio_test_bypass_ap tile0_example_ffd_usb_audio_test_bypass_ap tile1_example_ffd_usb_audio_test_bypass_ap 1)
merge_binaries(example_ffd_usb_audio_test_bypass_ap_dev tile0_example_ffd_usb_audio_test_bypass_ap_dev tile1_example_ffd_usb_audio_test_bypass_ap_dev 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_ffd_usb_audio_test)
create_debug_target(example_ffd_usb_audio_test)

create_run_target(example_ffd_usb_audio_test_dev)
create_debug_target(example_ffd_usb_audio_test_dev)

create_run_target(example_ffd_usb_audio_test_bypass_ap)
create_debug_target(example_ffd_usb_audio_test_bypass_ap)

create_run_target(example_ffd_usb_audio_test_bypass_ap_dev)
create_debug_target(example_ffd_usb_audio_test_bypass_ap_dev)
