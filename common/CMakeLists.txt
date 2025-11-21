
set(COMMON_H types.h serialization.h fileutils.h jsonutils.h)
set(COMMON_SRC fileutils.cpp)

add_library(common
            STATIC
            ${COMMON_H}
            ${COMMON_SRC})

set(APPLE_DEPS)
if (APPLE)
    set(APPLE_DEPS "-framework CoreServices")
endif()

target_link_libraries(common
                      PUBLIC
                      Boost::serialization
                      common_modules
                      PRIVATE
                      spdlog::spdlog
                      nlohmann_json::nlohmann_json
                      ${APPLE_DEPS}
                      )
