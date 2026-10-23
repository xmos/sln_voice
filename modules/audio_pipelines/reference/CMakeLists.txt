##******************************************
## Create fixed_delay AEC+IC+NS+AGC
##   2 mic input channels
#    2 reference input channels
##******************************************

add_library(fixed_delay_aec_ic_ns_agc_2mic_2ref INTERFACE)
target_sources(fixed_delay_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/fixed_delay/audio_pipeline_t0.c
        ${CMAKE_CURRENT_LIST_DIR}/fixed_delay/audio_pipeline_t1.c
)
target_include_directories(fixed_delay_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/fixed_delay
)
target_link_libraries(fixed_delay_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
        lib_voice::aec
        lib_voice::agc
        lib_voice::ic
        lib_voice::ns
        lib_voice::vnr
)

##******************************************
## Create ADEC "prevarch" AEC+IC+NS+AGC
##   2 mic input channels
#    2 reference input channels
##******************************************

add_library(adec_aec_ic_ns_agc_2mic_2ref INTERFACE)
target_sources(adec_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/adec/audio_pipeline_t0.c
        ${CMAKE_CURRENT_LIST_DIR}/adec/audio_pipeline_t1.c
)
target_include_directories(adec_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/adec
)
target_link_libraries(adec_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
        lib_voice::adec
        lib_voice::aec
        lib_voice::agc
        lib_voice::ic
        lib_voice::ns
        lib_voice::stage1
        lib_voice::vnr
)

##******************************************
## Create ADEC altarch AEC+IC+NS+AGC
##   2 mic input channels
#    2 reference input channels
##******************************************

add_library(adec_altarch_aec_ic_ns_agc_2mic_2ref INTERFACE)
target_sources(adec_altarch_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/adec_alt_arch/audio_pipeline_t0.c
        ${CMAKE_CURRENT_LIST_DIR}/adec_alt_arch/audio_pipeline_t1.c
)
target_include_directories(adec_altarch_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/adec_alt_arch
)
target_compile_definitions(adec_altarch_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ALT_ARCH_MODE=1
)
target_link_libraries(adec_altarch_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
        lib_voice::adec
        lib_voice::aec
        lib_voice::agc
        lib_voice::ic
        lib_voice::ns
        lib_voice::stage1
        lib_voice::vnr
)

##******************************************
## Create fixed delay altarch AEC+IC+NS+AGC
##   2 mic input channels
#    2 reference input channels
##******************************************

add_library(fixed_delay_altarch_aec_ic_ns_agc_2mic_2ref INTERFACE)
target_sources(fixed_delay_altarch_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/fixed_delay_alt_arch/audio_pipeline_t0.c
        ${CMAKE_CURRENT_LIST_DIR}/fixed_delay_alt_arch/audio_pipeline_t1.c
)
target_include_directories(fixed_delay_altarch_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/fixed_delay_alt_arch
)
target_compile_definitions(fixed_delay_altarch_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        ALT_ARCH_MODE=1
)
target_link_libraries(fixed_delay_altarch_aec_ic_ns_agc_2mic_2ref
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
        lib_voice::aec
        lib_voice::agc
        lib_voice::ic
        lib_voice::ns
        lib_voice::vnr
)

##******************************************
## Create empty audio pipeline
##******************************************

add_library(empty_2mic_2ref INTERFACE)
target_sources(empty_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/empty/audio_pipeline_t0.c
        ${CMAKE_CURRENT_LIST_DIR}/empty/audio_pipeline_t1.c
)
target_include_directories(empty_2mic_2ref
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/empty
)
target_link_libraries(empty_2mic_2ref
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
)

##*********************************************
## Create aliases for sln_voice example designs
##*********************************************

add_library(sln_voice::app::ffva::ap::adec_altarch ALIAS adec_altarch_aec_ic_ns_agc_2mic_2ref)
add_library(sln_voice::app::ffva::ap::fixed_delay ALIAS fixed_delay_aec_ic_ns_agc_2mic_2ref)
add_library(sln_voice::app::ffva::ap::fixed_delay_altarch ALIAS fixed_delay_altarch_aec_ic_ns_agc_2mic_2ref)
add_library(sln_voice::app::ffva::ap::adec ALIAS adec_aec_ic_ns_agc_2mic_2ref)
add_library(sln_voice::app::ffva::ap::empty ALIAS empty_2mic_2ref)
