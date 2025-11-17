
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

target_link_libraries(common
                      PUBLIC
                      Boost::serialization
                      common_modules
                      PRIVATE
                      spdlog::spdlog
                      nlohmann_json::nlohmann_json
                      ${APPLE_DEPS}
                      )
