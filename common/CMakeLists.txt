
set(COMMON_H types.h serialization.h jsonutils.h)
set(COMMON_SRC)

add_library(common
            STATIC
            ${COMMON_H}
            ${COMMON_SRC})

set(APPLE_DEPS)
if (APPLE)
    set(APPLE_DEPS "-framework CoreServices")
endif()

add_library(common_modules)
target_sources(common_modules
    PUBLIC
    FILE_SET CXX_MODULES FILES fileutils_module.cppm json_module.cppm)

target_link_libraries(common_modules
    PUBLIC
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Boost::serialization
)

target_link_libraries(common
                      PUBLIC
                      common_modules
                      PRIVATE
                      ${APPLE_DEPS}
                      )
