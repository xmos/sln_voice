
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    include(${CMAKE_CURRENT_LIST_DIR}/keyword_spotter/keyword_spotter.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/graphic_equaliser/graphic_equaliser.cmake)
endif()
