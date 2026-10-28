set(PROTOCOLS_H websocket_client.h trader.h strategy.h executor.h observer.h position_manager.h)
set(PROTOCOLS_SRC websocket_client.cpp trader.cpp strategy.cpp position_manager.cpp)

add_library(protocols STATIC ${PROTOCOLS_H} ${PROTOCOLS_SRC})

target_sources(protocols
  PUBLIC
    FILE_SET CXX_MODULES FILES
      risk_manager.cppm
)

target_link_libraries(protocols
                      PRIVATE
                      common
                      nlohmann_json::nlohmann_json
                      )
