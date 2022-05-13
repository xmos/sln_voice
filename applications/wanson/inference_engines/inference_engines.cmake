## Create wanson inference engine target
add_library(xcore_sdk_app_wanson_inference_engine_wanson INTERFACE)
target_sources(xcore_sdk_app_wanson_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng_port.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_inf_eng_support.c
)
target_include_directories(xcore_sdk_app_wanson_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/wanson
)
target_link_libraries(xcore_sdk_app_wanson_inference_engine_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/libasrengine_220511.a
        ${CMAKE_CURRENT_LIST_DIR}/wanson/libmodel_220511.a
)
target_compile_definitions(xcore_sdk_app_wanson_inference_engine_wanson
    INTERFACE
)

## Create an alias
add_library(sdk::app::inference_engine::wanson ALIAS xcore_sdk_app_wanson_inference_engine_wanson)
