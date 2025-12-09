find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc_doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

  add_dependencies(${PROJECT_NAME} doc_doxygen)
else()
    message(STATUS "Doxygen not found. Documentation target will not be available.")
endif()

function(enable_doxygen)
  option(ENABLE_DOXYGEN "Enable doxygen doc builds of source" ON)
    if (ENABLE_DOXYGEN)
        set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME} API Documentation" PARENT_SCOPE)
        set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/doc_doxygen" PARENT_SCOPE)
    endif()
endfunction()
