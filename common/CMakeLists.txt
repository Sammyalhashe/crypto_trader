
set(COMMON_H types.h serialization.h fileutils.h jsonutils.h)
set(COMMON_SRC fileutils.cpp)

add_library(common
            STATIC
            ${COMMON_H}
            ${COMMON_SRC})

target_link_libraries(common
                      PUBLIC
                      Boost::serialization
                      PRIVATE
                      spdlog::spdlog
                      nlohmann_json::nlohmann_json
                      )
