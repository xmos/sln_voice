add_library(asr_example STATIC)

target_sources(asr_example
    PRIVATE
        ${SOLUTION_VOICE_ROOT_PATH}/modules/asr/device_memory.c
        ${CMAKE_CURRENT_LIST_DIR}/asr_example_impl.c
)
target_include_directories(asr_example
    PUBLIC
        ${SOLUTION_VOICE_ROOT_PATH}/modules/asr
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

## Create an alias
add_library(sln_voice::app::asr::example ALIAS asr_example)