## Create custom ffd audio pipeline
add_library(app_ffd_audio_pipeline INTERFACE)

target_sources(app_ffd_audio_pipeline
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/audio_pipeline.c
)

target_include_directories(app_ffd_audio_pipeline
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(app_ffd_audio_pipeline
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
        fwk_voice::agc
        fwk_voice::ic
        fwk_voice::ns
        fwk_voice::vnr::features
        fwk_voice::vnr::inference
)

## Create an alias
add_library(sln_voice::app::ffd::ap ALIAS app_ffd_audio_pipeline)
