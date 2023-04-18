##******************************
## Create Wanson ASR port target
##******************************

add_library(asr_wanson INTERFACE)

target_sources(asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib_xcore_math_compat.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_asr_impl.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/flash_read_ext.c
)
target_include_directories(asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/wanson
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib
)
target_link_libraries(asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib/libasrengine_20220607.a
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib/libmodel_en_20220923.a
)

target_compile_definitions(asr_wanson
    INTERFACE
)

##*********************************************
## Create aliases for sln_voice example designs
##*********************************************

add_library(sln_voice::app::asr::wanson ALIAS asr_wanson)
