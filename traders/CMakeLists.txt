set(TRADERS_H CoinbaseTrader.h)
set(TRADERS_SRC CoinbaseTrader.cpp)

add_library(traders STATIC ${TRADERS_H} ${TRADERS_SRC})

target_link_libraries(traders
                      PRIVATE
                      protocols
                      strategies
                      Boost::system
                      openssl::openssl
                      nlohmann_json::nlohmann_json)
