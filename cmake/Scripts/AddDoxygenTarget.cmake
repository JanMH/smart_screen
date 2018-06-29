function(add_doxygen_target TARGET_NAME)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(${TARGET_NAME}
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )

    if(NOT TARGET doc)
        add_custom_target(doc)
    endif(NOT TARGET doc)
    add_dependencies(doc ${TARGET_NAME})

endif(DOXYGEN_FOUND)

endfunction(add_doxygen_target)

