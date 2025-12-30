option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)

if (ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if (CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
    else()
        message(SEND_ERROR "clang-tidy requested but executable not found")
    endif()
endif()

add_custom_target(cppcheck
    COMMAND cppcheck --enable=warning,style,performance,portability --suppress=missingIncludeSystem --suppress=unusedFunction --suppress=knownConditionTrueFalse --suppress=virtualCallInConstructor --suppress=noExplicitConstructor --suppress=missingOverride --suppress=shadowArgument --suppress=useStlAlgorithm --suppress=dangerousTypeCast --suppress=variableScope --suppress=unreadVariable --suppress=noConstructor --suppress=cstyleCast --suppress=unusedVariable --suppress=normalCheckLevelMaxBranches --std=c++20 --quiet --error-exitcode=1 ${CMAKE_SOURCE_DIR}/adaptors ${CMAKE_SOURCE_DIR}/common ${CMAKE_SOURCE_DIR}/databases ${CMAKE_SOURCE_DIR}/executors ${CMAKE_SOURCE_DIR}/protocols ${CMAKE_SOURCE_DIR}/strategies ${CMAKE_SOURCE_DIR}/traders
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running cppcheck..."
)

add_custom_target(style-check
    COMMAND python3 ${CMAKE_SOURCE_DIR}/style_checker.py adaptors common databases executors protocols strategies traders
    DEPENDS cppcheck
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running style checker..."
)
