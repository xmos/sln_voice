#**********************
# Gather Sources
#**********************

set(APP_SRC_PATH ${CMAKE_CURRENT_LIST_DIR}/src)
set(MIC_ARRAY_DEMO_PATH ${CMAKE_CURRENT_LIST_DIR}/../../modules/io/modules/mic_array/demos/)
set(PLATFORM_FILE ${APP_SRC_PATH}/XCORE-AI-EXPLORER.xn)


#We make a copy of the par decimator files to avoid include clashes from the demo
set(DEMO_PAR_DECIMATOR_FILES        ${MIC_ARRAY_DEMO_PATH}/demo_par_decimator/src/decimator_subtask.c
                                    ${MIC_ARRAY_DEMO_PATH}/demo_par_decimator/src/decimator_subtask.h
                                    ${MIC_ARRAY_DEMO_PATH}/demo_par_decimator/src/app_decimator.hpp
                                    ${MIC_ARRAY_DEMO_PATH}/demo_par_decimator/src/app_mic_array.hpp
)
file(COPY ${DEMO_PAR_DECIMATOR_FILES} DESTINATION ${APP_SRC_PATH}/par_decimator)

file(GLOB APP_SOURCES               ${APP_SRC_PATH}/*.c
                                    ${APP_SRC_PATH}/*.xc
                                    ${APP_SRC_PATH}/*.cpp
                                    ${APP_SRC_PATH}/par_decimator/*.c
                                    ${APP_SRC_PATH}/config.xscope
                                    ${MIC_ARRAY_DEMO_PATH}/common/src/device_pll_ctrl.c
)

# Add files for USB build config
message(STATUS CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR})
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/lib_xua.cmake)
    include("${CMAKE_CURRENT_LIST_DIR}/lib_xua.cmake")
endif()

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -O3
    -g
    -report
    -fxscope
    -mcmodel=large
    -Wall
    -Wno-xcore-fptrgroup
    ${PLATFORM_FILE}
)

set( APP_INCLUDES
    ${APP_SRC_PATH}
    ${XUA_INCLUDES}
    ${APP_SRC_PATH}/par_decimator
    ${MIC_ARRAY_DEMO_PATH}/common/src
)

set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    __xua_conf_h_exists__=1
    XUD_SERIES_SUPPORT=4
    USB_TILE=tile[1]
    XUD_CORE_CLOCK=600
)

set(APP_LINK_OPTIONS
    -report
    -lquadflash
    ${PLATFORM_FILE}
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_COMMON_LINK_LIBRARIES
    lib_i2s
    lib_i2c
    lib_mic_array
    lib_xud
)


#*************************
# Create Targets
#*************************
foreach(CONFIG tdm usb)
    set(TARGET_NAME example_mic_aggregator_${CONFIG})
    add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL )
    target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${XUA_SOURCES})
    target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} CONFIG_${CONFIG_UPPER}=1)
    target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
    target_link_libraries(${TARGET_NAME} PUBLIC ${APP_COMMON_LINK_LIBRARIES})
    target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
    unset(TARGET_NAME)

    # Copy output to a handy location
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.xe DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    unset(TARGET_NAME)
endforeach()
