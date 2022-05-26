
## Create keyword inference engine target
add_library(sln_avona_example_keyword_spotter_inference_engine_keyword INTERFACE)
target_sources(sln_avona_example_keyword_spotter_inference_engine_keyword
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_features.c
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_inference.cc
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_inference_port.c
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_model_data.c
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_model_labels.cc
)
target_include_directories(sln_avona_example_keyword_spotter_inference_engine_keyword
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/keyword
)
target_link_libraries(sln_avona_example_keyword_spotter_inference_engine_keyword
    INTERFACE
        core::microfrontend
        rtos::sw_services::inferencing
)
target_compile_definitions(sln_avona_example_keyword_spotter_inference_engine_keyword
    INTERFACE
)

## Create an alias
add_library(sln_avona::example::inference_engine::keyword ALIAS sln_avona_example_keyword_spotter_inference_engine_keyword)
