
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL XCORE_XS3A)
    message(STATUS, "CURRENT list directory = ${CMAKE_CURRENT_LIST_DIR}")
    add_library(src INTERFACE)

    target_include_directories(src
        INTERFACE
            api
    )
    file(GLOB SRC_SOURCES "src/*.c" "src/asm/*.S")
    target_sources(src INTERFACE ${SRC_SOURCES})
    add_library(fwk_xvf::src ALIAS src)
endif()
