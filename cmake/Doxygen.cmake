find_package(Doxygen REQUIRED) # Make Doxygen finding mandatory
find_package(Graphviz REQUIRED) # Make Graphviz finding mandatory

# Define variables for configure_file in this scope
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/doc_doxygen")
set(GRAPHVIZ_DOT_EXECUTABLE_PATH "${GRAPHVIZ_DOT_EXECUTABLE}")


if(DOXYGEN_FOUND AND GRAPHVIZ_FOUND)
    # Removed debug messages: message(STATUS "Doxygen found: ${DOXYGEN_FOUND}, Executable: ${DOXYGEN_EXECUTABLE}")
    # Removed debug messages: message(STATUS "Graphviz found: ${GRAPHVIZ_FOUND}, dot Executable: ${GRAPHVIZ_DOT_EXECUTABLE}")

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
    message(FATAL_ERROR "Doxygen or Graphviz not found by CMake. Please ensure they are installed and in PATH.")
endif()

function(enable_doxygen)
  option(ENABLE_DOXYGEN "Enable doxygen doc builds of source" ON)
    if (ENABLE_DOXYGEN)
        set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME} API Documentation" PARENT_SCOPE)
        # OUTPUT_DIRECTORY and DOT_EXECUTABLE_PATH are passed directly to configure_file.
    endif()
endfunction()