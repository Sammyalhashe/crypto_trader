cmake_minimum_required(VERSION 3.23)

project(crypto_trader C CXX)

# specify the cmake standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# This is required if we want to do a debug build and build against Release
# build conan packages.
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG RELEASE)

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

# project definitions below
find_package(fmt REQUIRED)

# all headers
set(TARGET_H
    )

# all cpps
set(TARGET_SRC
    main.cpp)

# add the actual executable
add_executable(crypto_trader ${TARGET_H} ${TARGET_SRC})


# handle all subdirectories
set(subdirs strategies test)
foreach(dir strategies test)
    add_subdirectory(${dir})
endforeach()
target_link_directories(${PROJECT_NAME} PRIVATE ${subdirs})

# link everything together
target_link_libraries(${PROJECT_NAME}
                      PRIVATE
                      ${project_warnings}
                      ${subdirs}
                      fmt::fmt)
