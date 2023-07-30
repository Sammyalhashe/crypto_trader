function(set_project_warnings project_name)
    option(WARNING_AS_ERRORS "Treat compiler warnings as errors" TRUE)

    
    # if compiling with clang
    set(CLANG_WARNINGS
        -Wall
        -Wextra             # reasonable and standard
        -Wshadow            # warns whenever a variable shadows another
        -Wnon-virtual-dtor  # self-explanatory
        -Wunused
        -Wold-style-cast
        -Wcast-align
        -Woverloaded-virtual
        -Wformat=2          # warn on security issues around functions that
                            # format output
    )

    # if compiling with gcc
    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wduplicated-branches # warn if if/else branches have duplicated code
        -Wlogical-op          # warn if logical ops are used where bitwise ops are likely wanted
        -Wuseless-cast        # warn if you perform a cast to the same type
    )

    if (WARNINGS_AS_ERRORS)
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
    endif()


    # actually set the project warnings
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message("compiler detected as Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif ()
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    endif()

    target_compile_options(${project_name} INTERFACE ${PROJECT_WARNINGS})

endfunction()
