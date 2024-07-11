##******************************************
## Create IC+NS+AGC
##   2 mic input channels
##******************************************

add_library(ic_ns_agc_2mic INTERFACE)

target_sources(ic_ns_agc_2mic
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ic_ns_agc/audio_pipeline.c
)

target_include_directories(ic_ns_agc_2mic
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/ic_ns_agc
)

target_link_libraries(ic_ns_agc_2mic
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

##******************************************
## Create empty audio pipeline
##******************************************

add_library(empty_2mic INTERFACE)

target_sources(empty_2mic
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/empty/audio_pipeline.c
)
target_include_directories(empty_2mic
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/empty
)
target_link_libraries(empty_2mic
    INTERFACE
        core::general
        rtos::freertos
        rtos::sw_services::generic_pipeline
)

##*********************************************
## Create aliases for sln_voice example designs
##*********************************************

add_library(sln_voice::app::ffd::ap::ic_ns_agc ALIAS ic_ns_agc_2mic)
add_library(sln_voice::app::ffd::ap::empty ALIAS empty_2mic)
