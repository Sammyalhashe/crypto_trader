cmake_minimum_required(VERSION 3.10)

project(crypto_trader C CXX)

if(APPLE)
    message(STATUS "FORCE ENABLING C++20 CONCURRENCY FOR MACOS")
    # 1. Disable the "Your OS is too old for this" check
    add_compile_definitions(_LIBCPP_DISABLE_AVAILABILITY)
    
    # 2. Force the headers to reveal jthread and stop_token
    add_compile_definitions(__cpp_lib_jthread=201907L)
    add_compile_definitions(__cpp_lib_stop_token=201907L)
    
    # 3. Some versions of libc++ require this for experimental features
    add_compile_options(-fexperimental-library)

    # 4. Target specific OSX version at least
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum OS X version" FORCE)
endif()

# specify the cmake standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# if a conan build or not
SET(CACHE{CONAN} BOOL "If in a Conan build" VALUE true false)

# use ccache to improve build times
find_program(CCACHE ccache)
if(CCACHE)
    message("using ccache....")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
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

# add custom functions
include(cmake/Functions.cmake)

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
find_package(SQLiteCpp REQUIRED)

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
set(subdirs traders strategies common protocols adaptors databases executors)
foreach(dir ${subdirs})
    message("\tadding dir ${dir}...")
    add_subdirectory(${dir})
endforeach()

# zig
set(ZIG_LIBS "")
make_zig_lib(ZIG_LIBS ${PROJECT_NAME} zigmath zig/zigmath)

# link everything together
message("CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
target_link_libraries(${PROJECT_NAME}
                      PRIVATE
                      ${project_warnings}
                      ${project_flags}
                      ${subdirs}
                      ${lib_deps}
                      ${ZIG_LIBS})

# testing
enable_testing()
include(GoogleTest)

function(create_test name filepath flags)
    add_executable(${name} ${filepath})
    target_link_libraries(${name}
                          GTest::gtest_main
                          GTest::gmock_main
                          Boost::system
                          nlohmann_json::nlohmann_json
                          spdlog::spdlog
                          ${subdirs}
                          ${flags})
    gtest_discover_tests(${name})
endfunction()

# common util tests
create_test(fileutil_test common/fileutils.t.cpp project_flags)
create_test(accounting_test common/Accounting.t.cpp project_flags)
create_test(common_math_test common/math.t.cpp project_flags)
# strategy tests
create_test(simple_test strategies/simple.t.cpp project_flags)
create_test(hodl_test strategies/hodl.t.cpp project_flags)
create_test(database_test databases/market_data_db.t.cpp project_flags)

# zig tests
add_zig_test_module(math_test zig/zigmath/math.zig)
