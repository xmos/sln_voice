set(TARGET_NAME example_asrc_fileio)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/asrc_app.c
        ${CMAKE_CURRENT_LIST_DIR}/src/asrc_process.c
        ${CMAKE_CURRENT_LIST_DIR}/src/i2s_task.c
        ${CMAKE_CURRENT_LIST_DIR}/src/main.xc
        ${CMAKE_CURRENT_LIST_DIR}/src/file_utils/fileio.c
        ${CMAKE_CURRENT_LIST_DIR}/src/file_utils/wav_utils.c
        )

target_include_directories(${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/file_utils)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        xscope_fileio
        lib_src
        lib_i2s
)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
        TEST_WAV_XSCOPE=1
    )

target_compile_options(${TARGET_NAME}
    PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
        -Os
        -g
        -report
        -mcmodel=large
    )

target_link_options(${TARGET_NAME}
    PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
        ${CMAKE_CURRENT_LIST_DIR}/config.xscope
        -report
        -mcmodel=large
    )

