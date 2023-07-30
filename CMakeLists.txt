cmake_minimum_required(VERSION 3.23)

project(crypto_trader C CXX)

# specify the cmake standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# use ccache to improve build times
find_program(CCACHE ccache)
if(CCACHE)
    message("using ccache....")
    set(CMAKE_CXX_COMPILER_LAUNCHERS ${CCACHE})
else()
    message("ccache not found")
endif()

# export a compile commands for lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# standard compiler warnings
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# setup static analysis
include(cmake/StaticAnalyzers.cmake)

# doxygen support
include(cmake/Doxygen.cmake)
enable_doxygen()
