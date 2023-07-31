
## Create custom board targets for dacs
set(TARGET_NAME sln_voice_app_ffva_asrc_otg_board_support_dac_aic3204)
add_library(${TARGET_NAME} INTERFACE)
target_sources(${TARGET_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/aic3204/aic3204.c
)
target_include_directories(${TARGET_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/aic3204
)
target_compile_definitions(${TARGET_NAME}
    INTERFACE
        AIC3204=1
)

## Create an alias
add_library(sln_voice::app::ffva_asrc_otg::dac::aic3204 ALIAS ${TARGET_NAME})

set(TARGET_NAME_3103 sln_voice_app_ffva_asrc_otg_board_support_dac_dac3103)
## Create custom board targets for dacs
add_library(${TARGET_NAME_3103} INTERFACE)
target_sources(${TARGET_NAME_3103}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/dac3101/dac3101.c
)
target_include_directories(${TARGET_NAME_3103}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/dac3101
)
target_compile_definitions(${TARGET_NAME_3103}
    INTERFACE
        DAC3101=1
)

## Create an alias
add_library(sln_voice::app::ffva_asrc_otg::dac::dac3101 ALIAS ${TARGET_NAME_3103})
