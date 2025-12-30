set(COMMON_H types.h serialization.h fileutils.h jsonutils.h timeutils.h Event.h Accounting.h math.h)
set(COMMON_SRC fileutils.cpp Accounting.cpp)

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
                      PRIVATE
                      spdlog::spdlog
                      nlohmann_json::nlohmann_json
                      ${APPLE_DEPS}
                      )

target_include_directories(common PUBLIC ${nlohmann_json_INCLUDE_DIR})


