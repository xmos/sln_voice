if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(custom_framework_rtos_drivers_i2s INTERFACE)
    target_sources(custom_framework_rtos_drivers_i2s
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/rtos_i2s.c
        ${CMAKE_CURRENT_LIST_DIR}/../../../../modules/rtos/modules/drivers/i2s/src/rtos_i2s_rpc.c
    )
    target_include_directories(custom_framework_rtos_drivers_i2s
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
    )
    target_link_libraries(custom_framework_rtos_drivers_i2s
        INTERFACE
            lib_i2s
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::custom_i2s_with_rate_calc ALIAS custom_framework_rtos_drivers_i2s)
endif()
