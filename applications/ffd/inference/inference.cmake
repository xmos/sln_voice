## Create wanson inference engine target
add_library(sln_voice_app_ffd_inference_engine_wanson INTERFACE)
target_sources(sln_voice_app_ffd_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng_port.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng_support.c
)
target_include_directories(sln_voice_app_ffd_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/wanson
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib
)
target_link_libraries(sln_voice_app_ffd_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib/libasrengine_20220607.a
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib/libmodel_en_20220607.a
)
target_compile_definitions(sln_voice_app_ffd_inference_engine_wanson
    INTERFACE
)

## Create an alias
add_library(sln_voice::app::ffd::inference_engine::wanson ALIAS sln_voice_app_ffd_inference_engine_wanson)
