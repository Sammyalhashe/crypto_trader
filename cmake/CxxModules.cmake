# This file handles the necessary configuration to support C++20 modules,
# particularly in complex environments like Nix where standard system include
# paths might not be automatically detected by CMake's dependency scanner.

# -----------------------------------------------------------------------------
# Dynamic Include Path Discovery for clang-scan-deps
# -----------------------------------------------------------------------------
# Issue:
# When building C++20 modules, CMake uses `clang-scan-deps` to determine the
# dependency order of modules. In some environments (like Nix shells or certain
# cross-compilation setups), the compiler wrapper handles implicit include paths
# (e.g., for the C++ standard library), but these implicit paths are NOT
# automatically passed to `clang-scan-deps`.
#
# Result:
# This leads to build failures where the scanner cannot find standard headers
# like <iostream>, <vector>, or <string_view>, even though the actual compilation
# step works fine.
#
# Solution:
# We explicitly ask the compiler for its implicit include directories by running
# it with `-E -x c++ - -v`. We parse the output to find the search paths and
# then populate `CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES`. This variable ensures
# that CMake passes these paths to the dependency scanner, resolving the issue
# without hardcoding specific paths (which would break across different machines
# or Nix generations).
# -----------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Detecting implicit C++ include directories for module scanning...")
    
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -E -x c++ - -v
        INPUT_FILE /dev/null
        ERROR_VARIABLE COMPILER_OUTPUT
        OUTPUT_QUIET
    )

    string(REPLACE "\n" ";" COMPILER_LINES "${COMPILER_OUTPUT}")
    set(CXX_IMPLICIT_PATHS "")
    set(COLLECT_PATHS FALSE)

    foreach(LINE ${COMPILER_LINES})
        # Start collecting when we see the start marker
        if(LINE MATCHES "#include <...> search starts here:")
            set(COLLECT_PATHS TRUE)
        # Stop collecting when we see the end marker
        elseif(LINE MATCHES "End of search list.")
            set(COLLECT_PATHS FALSE)
        # Process lines if we are in the collection block
        elseif(COLLECT_PATHS)
            string(STRIP "${LINE}" STRIPPED_LINE)
            # Filter out empty lines and macOS framework directory markers
            if(STRIPPED_LINE AND NOT STRIPPED_LINE MATCHES " \(framework directory\)$")
                list(APPEND CXX_IMPLICIT_PATHS "${STRIPPED_LINE}")
            endif()
        endif()
    endforeach()

    if(CXX_IMPLICIT_PATHS)
        list(REMOVE_DUPLICATES CXX_IMPLICIT_PATHS)
        message(STATUS "Setting CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES to: ${CXX_IMPLICIT_PATHS}")
        set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CXX_IMPLICIT_PATHS})
    else()
        message(WARNING "Failed to detect implicit C++ include directories. Module scanning may fail.")
    endif()
endif()
