find_package(Doxygen REQUIRED) # Make Doxygen finding mandatory

# Define variables for configure_file in this scope
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/doc_doxygen")
# GRAPHVIZ_DOT_EXECUTABLE will be provided by find_package(Graphviz REQUIRED)
set(GRAPHVIZ_DOT_EXECUTABLE_PATH "${GRAPHVIZ_DOT_EXECUTABLE}")


if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    if (EXISTS "${DOXYGEN_IN}")
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    else()
        message(FATAL_ERROR "Doxyfile.in not found at: ${DOXYGEN_IN}")
    endif()

    add_custom_target(doc_doxygen
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        # Removed: WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

else()
    message(FATAL_ERROR "Doxygen not found by CMake. Please ensure it is installed and in PATH.")
endif()

function(enable_doxygen)
  option(ENABLE_DOXYGEN "Enable doxygen doc builds of source" ON)
    if (ENABLE_DOXYGEN)
        set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME} API Documentation" PARENT_SCOPE)
        # OUTPUT_DIRECTORY and DOT_EXECUTABLE_PATH are passed directly to configure_file.
    endif()
endfunction()
