## Create wanson inference engine target
add_library(sln_avona_app_ffd_inference_engine_wanson INTERFACE)
target_sources(sln_avona_app_ffd_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng_port.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng_support.c
)
target_include_directories(sln_avona_app_ffd_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/wanson
)
target_link_libraries(sln_avona_app_ffd_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/libasrengine_20220519.a
        ${CMAKE_CURRENT_LIST_DIR}/wanson/libmodel_en_20220520.a
)
target_compile_definitions(sln_avona_app_ffd_inference_engine_wanson
    INTERFACE
)

## Create an alias
add_library(sln_avona::app::ffd::inference_engine::wanson ALIAS sln_avona_app_ffd_inference_engine_wanson)
