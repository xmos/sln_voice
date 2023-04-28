##*******************************
## Create Sensory ASR port target
##*******************************

add_library(asr_sensory INTERFACE)

target_sources(asr_sensory
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/device_memory.c
        ${CMAKE_CURRENT_LIST_DIR}/sensory/appAudio.c
        ${CMAKE_CURRENT_LIST_DIR}/sensory/sensory_asr.c
)
target_include_directories(asr_sensory
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/sensory
        ${CMAKE_CURRENT_LIST_DIR}/sensory/lib
)
## suppress all linker warnings
target_link_options(asr_sensory 
    INTERFACE 
        -Wl,-w
)
target_link_libraries(asr_sensory
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/sensory/lib/libTHFMicro-7.2.a
)
target_compile_definitions(asr_sensory
    INTERFACE
)

##******************************
## Create Wanson ASR port target
##******************************

add_library(asr_wanson INTERFACE)

target_sources(asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/flash_read_ext.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib_xcore_math_compat.c
        ${CMAKE_CURRENT_LIST_DIR}/wanson/wanson_asr.c
)
target_include_directories(asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/wanson
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib
)
## suppress all linker warnings
target_link_options(asr_wanson 
    INTERFACE 
        -Wl,-w
)
target_link_libraries(asr_wanson
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib/libasrengine_20220607.a
        ${CMAKE_CURRENT_LIST_DIR}/wanson/lib/libmodel_en_20220923.a
)

target_compile_definitions(asr_wanson
    INTERFACE
)
##*********************************************
## Create aliases for sln_voice example designs
##*********************************************

add_library(sln_voice::app::asr::sensory ALIAS asr_sensory)
add_library(sln_voice::app::asr::wanson ALIAS asr_wanson)
