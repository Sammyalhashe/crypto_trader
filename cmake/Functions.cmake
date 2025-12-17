function(add_zig_test_module name module_zig_file)
    add_test(
        NAME ${name}
        COMMAND zig test ${CMAKE_CURRENT_SOURCE_DIR}/${module_zig_file} -OReleaseSafe
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

function(make_zig_lib OUT_LIST_VAR TARGET_NAME LIB_NAME ZIG_DIR)
    # Paths
    set(ZIG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${ZIG_DIR}")
    set(ZIG_OUT_DIR "${ZIG_SRC_DIR}/zig-out/lib")
    # CMAKE_SHARED_LIBRARY_SUFFIX handles .so vs .dylib automatically
    set(ZIG_LIB "${ZIG_OUT_DIR}/lib${LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")

    # Find all .zig source files in the directory recursively
    file(GLOB_RECURSE ZIG_SOURCES "${ZIG_SRC_DIR}/*.zig")

    # Custom command: builds the Zig dynamic library
    if (APPLE)
        add_custom_command(
            OUTPUT ${ZIG_LIB}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${ZIG_OUT_DIR}
            COMMAND zig build -Dtarget=aarch64-macos.12.0
            WORKING_DIRECTORY ${ZIG_SRC_DIR}
            DEPENDS ${ZIG_SOURCES}        # <-- now CMake knows about all Zig files
            COMMENT "Building Zig dynamic library: ${LIB_NAME}"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT ${ZIG_LIB}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${ZIG_OUT_DIR}
            COMMAND zig build
            WORKING_DIRECTORY ${ZIG_SRC_DIR}
            DEPENDS ${ZIG_SOURCES}        # <-- now CMake knows about all Zig files
            COMMENT "Building Zig dynamic library: ${LIB_NAME}"
            VERBATIM
        )
    endif()

    # Custom target that depends on the built library
    add_custom_target(zig-${LIB_NAME}-build ALL
        DEPENDS ${ZIG_LIB}
    )

    # Make the C++ target depend on the Zig library
    add_dependencies(${TARGET_NAME} zig-${LIB_NAME}-build)

    # Export the library path to the output list variable
    list(APPEND ${OUT_LIST_VAR} ${ZIG_LIB})
    set(${OUT_LIST_VAR} ${${OUT_LIST_VAR}} PARENT_SCOPE)

    # Include generated Zig headers if they exist
    set(ZIG_INCLUDE_DIR "${ZIG_SRC_DIR}/zig-out/include")
    if(EXISTS ${ZIG_INCLUDE_DIR})
        target_include_directories(${TARGET_NAME} PRIVATE ${ZIG_INCLUDE_DIR})
    endif()
endfunction()
