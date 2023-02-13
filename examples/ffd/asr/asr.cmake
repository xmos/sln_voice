#**************************
# Wanson ASR port target
#**************************

add_library(sln_voice_app_ffd_asr_wanson INTERFACE)

target_sources(sln_voice_app_ffd_asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/port/wanson/lib_xcore_math_compat.c
        ${CMAKE_CURRENT_LIST_DIR}/port/wanson/wanson_asr_impl.c
        ${CMAKE_CURRENT_LIST_DIR}/port/wanson/flash_read_ext.c
)
target_include_directories(sln_voice_app_ffd_asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/port/wanson
        ${CMAKE_CURRENT_LIST_DIR}/port/wanson/lib
)
target_link_libraries(sln_voice_app_ffd_asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/port/wanson/lib/libasrengine_20220607.a
        ${CMAKE_CURRENT_LIST_DIR}/port/wanson/lib/libmodel_en_20220923.a
)

target_compile_definitions(sln_voice_app_ffd_asr_wanson
    INTERFACE
)

## Create an alias
add_library(sln_voice::app::ffd::asr::wanson ALIAS sln_voice_app_ffd_asr_wanson)
