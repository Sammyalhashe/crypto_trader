set(STRATEGIES_H index.h simple.h hodl.h)
set(STRATEGIES_SRC simple.cpp hodl.cpp)

add_library(strategies STATIC ${STRATEGIES_H} ${STRATEGIES_SRC})

target_link_libraries(strategies
                      PRIVATE
                      protocols
                      Boost::system
                      openssl::openssl
                      nlohmann_json::nlohmann_json
                      spdlog::spdlog
                      )
