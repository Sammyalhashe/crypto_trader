set(PROTOCOLS_H websocket_client.h trader.h strategy.h)
set(PROTOCOLS_SRC websocket_client.cpp strategy.cpp)

add_library(protocols STATIC ${PROTOCOLS_H} ${PROTOCOLS_SRC})

target_link_libraries(protocols
                      PRIVATE
                      common
                      nlohmann_json::nlohmann_json
                      )
