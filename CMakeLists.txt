cmake_minimum_required(VERSION 3.28)

project(crypto_trader C CXX)

# specify the cmake standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# if a conan build or not
SET(CACHE{CONAN} BOOL "If in a Conan build" VALUE true false)

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

# other compiler options
add_library(project_flags INTERFACE)
set(SANITIZE "")
message("sanitize_flag: ${custom_build_type}")
if (custom_build_type)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=${custom_build_type}")
    set(SANITIZE "${CMAKE_CXX_FLAGS} -fsanitize=${custom_build_type}")
endif()

# See https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/CppRuntimeEnv/Articles/SymbolVisibility.html
# and https://stackoverflow.com/questions/8685045/xcode-with-boost-linkerid-warning-about-visibility-settings/11879361#11879361
# Also, there is a bug in clang and it specifically affects boost hence the
# disabling of concepts in boost.
if (APPLE)
    set(CMAKE_CXX_FLAGS "-fvisibility=hidden -DBOOST_ASIO_DISABLE_CONCEPTS")
endif()

target_compile_options(project_flags INTERFACE ${SANITIZE})

# setup static analysis
include(cmake/StaticAnalyzers.cmake)

# doxygen support
include(cmake/Doxygen.cmake)
enable_doxygen()

# project dependencies below
find_package(OpenSSL REQUIRED)
find_package(Boost CONFIG 1.87.0 COMPONENTS thread system serialization REQUIRED)
find_package(spdlog 1.12.0 REQUIRED)

# NOTE: This may not be needed.
if (Boost_Found)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(nlohmann_json 3.11.2 REQUIRED)
find_package(GTest REQUIRED)

# all headers
set(TARGET_H)

# all cpps
set(TARGET_SRC
    main.cpp)

# library dependencies
if(CONAN)
    set(lib_deps
        Boost::system
        Boost::thread
        nlohmann_json::nlohmann_json
        openssl::openssl
        spdlog::spdlog)
else()
    set(lib_deps
        Boost::system
        Boost::thread
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        spdlog::spdlog)
endif()

# add the actual executable
add_executable(${PROJECT_NAME} ${TARGET_H} ${TARGET_SRC})

# handle all subdirectories
set(subdirs traders strategies common protocols adaptors databases)
foreach(dir ${subdirs})
    message("\tadding dir ${dir}...")
    add_subdirectory(${dir})
endforeach()

# target_link_directories(adaptors PRIVATE protocols)
# target_link_directories(${PROJECT_NAME} PRIVATE ${subdirs})

# link everything together
message("CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
target_link_libraries(${PROJECT_NAME}
                      PRIVATE
                      ${project_warnings}
                      ${project_flags}
                      ${subdirs}
                      ${lib_deps})

# testing
enable_testing()
include(GoogleTest)

function(add_test name filepath flags)
    add_executable(${name} ${filepath})
    target_link_libraries(${name}
                          ${FOUNDATION}
                          GTest::gtest_main
                          Boost::system
                          nlohmann_json::nlohmann_json
                          spdlog::spdlog
                          ${subdirs}
                          ${flags})
    gtest_discover_tests(${name})
endfunction()

# common util tests
add_test(fileutil_test common/fileutils.t.cpp project_flags)
# strategy tests
add_test(simple_test strategies/simple.t.cpp project_flags)
add_test(hodl_test strategies/hodl.t.cpp project_flags)
add_test(database_test databases/market_data_db.t.cpp project_flags)
