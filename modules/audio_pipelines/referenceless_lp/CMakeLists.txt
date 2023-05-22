##******************************************
## Create IC+NS+AGC 
##   2 mic input channels
#    2 reference input channels
##******************************************

add_library(ic_ns_agc_2mic_2ref_lp INTERFACE)

target_sources(ic_ns_agc_2mic_2ref_lp
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/audio_pipeline.c
)

target_include_directories(ic_ns_agc_2mic_2ref_lp
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(ic_ns_agc_2mic_2ref_lp
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

##*********************************************
## Create aliases for sln_voice example designs
##*********************************************

add_library(sln_voice::app::ffd::ap_lp ALIAS ic_ns_agc_2mic_2ref_lp)
