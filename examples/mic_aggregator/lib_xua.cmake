# This file picks the sources and adds include files for the USB version of mic_aggregator
# Note lib_xua is not currently a cmake supported lib so this file works around that

set(XUA_SRC_PATH ${CMAKE_CURRENT_LIST_DIR}/../../modules/xua/lib_xua)

file(GLOB_RECURSE XUA_SOURCES       ${XUA_SRC_PATH}/src/core/buffer/*.xc
                                    ${XUA_SRC_PATH}/src/core/buffer/*.c
                                    ${XUA_SRC_PATH}/src/core/endpoint0/*.xc
                                    ${XUA_SRC_PATH}/src/core/endpoint0/*.c
                                    ${XUA_SRC_PATH}/src/core/user/*.c
                                    ${XUA_SRC_PATH}/src/core/support/*.xc
                                    ${XUA_SRC_PATH}/src/dfu/*.c
                                    ${XUA_SRC_PATH}/src/dfu/*.xc
)

set(XUA_INCLUDES                    ${XUA_SRC_PATH}/api
                                    ${XUA_SRC_PATH}/src/dfu
                                    ${XUA_SRC_PATH}/src/core
                                    ${XUA_SRC_PATH}/src/core/audiohub
                                    ${XUA_SRC_PATH}/src/core/endpoint0
                                    ${XUA_SRC_PATH}/src/core/support
                                    ${XUA_SRC_PATH}/src/core/user/audiostream
                                    ${XUA_SRC_PATH}/src/core/user/hostactive
                                    ${XUA_SRC_PATH}/src/hid
)
