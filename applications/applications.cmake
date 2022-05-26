
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    include(${CMAKE_CURRENT_LIST_DIR}/stlp/stlp.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/ffd/ffd.cmake)
else()
    #include(${CMAKE_CURRENT_LIST_DIR}/stlp/host)
endif()
