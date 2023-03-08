add_library(asr_example STATIC)

target_sources(asr_example
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/asr_example_impl.c
)
target_include_directories(asr_example
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../../api
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

## Create an alias
add_library(sln_voice::asr_example ALIAS asr_example)