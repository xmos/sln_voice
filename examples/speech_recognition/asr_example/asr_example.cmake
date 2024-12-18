add_library(asr_example STATIC)

target_sources(asr_example
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/asr_example_impl.c
        ${SOLUTION_VOICE_ROOT_PATH}/modules/asr/device_memory/device_memory.c
)
target_include_directories(asr_example
    PUBLIC
        ${SOLUTION_VOICE_ROOT_PATH}/modules/asr
        ${SOLUTION_VOICE_ROOT_PATH}/modules/asr/device_memory
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

## Create an alias
add_library(sln_voice::app::asr::example ALIAS asr_example)