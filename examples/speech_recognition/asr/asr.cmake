add_library(sln_voice_asr INTERFACE)

target_sources(sln_voice_asr
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/port/mock/mock_asr_impl.c
)
target_include_directories(sln_voice_asr
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
)

## Create an alias
add_library(sln_voice::asr ALIAS sln_voice_asr)