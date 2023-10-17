

set(ASRC_EXAMPLE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../examples/asrc_demo)

set(CMAKE_OSX_ARCHITECTURES "" CACHE INTERNAL "")

add_executable(test_asrc_div
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pseudo_rand.c
    ${ASRC_EXAMPLE_PATH}/src/shared/div.c
)

target_include_directories(test_asrc_div
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${ASRC_EXAMPLE_PATH}/src/shared
)


target_link_libraries( test_asrc_div PRIVATE lib_xcore_math )

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    target_compile_options(test_asrc_div
        PRIVATE "-target=XCORE-AI-EXPLORER")

    target_link_options(test_asrc_div
        PRIVATE
            "-target=XCORE-AI-EXPLORER"
            "-report")
else()
    target_link_libraries(test_asrc_div
        PRIVATE m)
    target_compile_definitions(test_asrc_div PRIVATE X86_BUILD=1)
endif()
