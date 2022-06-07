#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES  
    ${CMAKE_CURRENT_LIST_DIR}/src/*.xc 
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c 
)

set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/equaliser
)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -g
    -report
    -fxscope
    -Wno-xcore-fptrgroup
    -target=XCORE-AI-EXPLORER
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_SUPPORTS_TILE_0=1
    PLATFORM_SUPPORTS_TILE_1=1
    PLATFORM_SUPPORTS_TILE_2=0
    PLATFORM_SUPPORTS_TILE_3=0
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=0
)

set(APP_LINK_OPTIONS
    -fxscope
    -report
    -target=XCORE-AI-EXPLORER
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
add_executable(example_graphic_equaliser EXCLUDE_FROM_ALL)
target_sources(example_graphic_equaliser PUBLIC ${APP_SOURCES})
target_include_directories(example_graphic_equaliser PUBLIC ${APP_INCLUDES})
target_compile_definitions(example_graphic_equaliser PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(example_graphic_equaliser PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(example_graphic_equaliser PUBLIC core::dsp_filters)
target_link_options(example_graphic_equaliser PRIVATE ${APP_LINK_OPTIONS})
