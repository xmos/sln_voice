## Create custom stlp audiopipeline
add_library(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms INTERFACE)
target_sources(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/fixed_delay/audio_pipeline_t0.c
        ${CMAKE_CURRENT_LIST_DIR}/src/fixed_delay/audio_pipeline_t1.c
        ${CMAKE_CURRENT_LIST_DIR}/src/fixed_delay/aec/aec_process_frame_1thread.c
)
target_include_directories(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/src/fixed_delay
)
target_link_libraries(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
        fwk_voice::aec
        fwk_voice::agc
        fwk_voice::ic
        fwk_voice::ns
        fwk_voice::vnr::features
        fwk_voice::vnr::inference
)

## Create an alias
add_library(sln_voice::app::stlp::ap::fixed_delay ALIAS xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms)


## Create custom stlp audiopipeline
add_library(sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms INTERFACE)
target_sources(sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/adec/audio_pipeline_t0.c
        ${CMAKE_CURRENT_LIST_DIR}/src/adec/audio_pipeline_t1.c
        ${CMAKE_CURRENT_LIST_DIR}/src/adec/stage1/delay_buffer.c
        ${CMAKE_CURRENT_LIST_DIR}/src/adec/stage1/stage_1.c
        ${CMAKE_CURRENT_LIST_DIR}/src/adec/aec/aec_process_frame_1thread.c
)
target_include_directories(sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/src/adec
        ${CMAKE_CURRENT_LIST_DIR}/src/adec/aec
        ${CMAKE_CURRENT_LIST_DIR}/src/adec/stage1
)
target_link_libraries(sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
        fwk_voice::adec
        fwk_voice::aec
        fwk_voice::agc
        fwk_voice::ic
        fwk_voice::ns
        fwk_voice::vnr::features
        fwk_voice::vnr::inference
)

## Create an alias
add_library(sln_voice::app::stlp::ap::adec ALIAS sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms)

## Create custom stlp audiopipeline
add_library(sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms_altarch INTERFACE)
target_sources(sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms_altarch
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/adec_alt_arch/audio_pipeline_t0.c
        ${CMAKE_CURRENT_LIST_DIR}/src/adec_alt_arch/audio_pipeline_t1.c
        ${CMAKE_CURRENT_LIST_DIR}/src/adec_alt_arch/stage1/delay_buffer.c
        ${CMAKE_CURRENT_LIST_DIR}/src/adec_alt_arch/stage1/stage_1.c
        ${CMAKE_CURRENT_LIST_DIR}/src/adec_alt_arch/aec/aec_process_frame_1thread.c
)
target_include_directories(sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms_altarch
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/src/adec_alt_arch
        ${CMAKE_CURRENT_LIST_DIR}/src/adec_alt_arch/aec
        ${CMAKE_CURRENT_LIST_DIR}/src/adec_alt_arch/stage1
)
target_link_libraries(sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms_altarch
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
        fwk_voice::adec
        fwk_voice::aec
        fwk_voice::agc
        fwk_voice::ic
        fwk_voice::ns
        fwk_voice::vnr::features
        fwk_voice::vnr::inference
)

## Create an alias
add_library(sln_voice::app::stlp::ap::adec_altarch ALIAS sln_voice_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms_altarch)
