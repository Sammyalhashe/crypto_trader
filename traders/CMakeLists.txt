set(TRADERS_H CoinbaseTrader.h)
set(TRADERS_SRC CoinbaseTrader.cpp)

add_library(traders STATIC ${TRADERS_H} ${TRADERS_SRC})

if(CONAN)
    target_link_libraries(traders
                          PRIVATE
                          protocols
                          common
                          databases
                          strategies
                          Boost::system
                          openssl::openssl
                          nlohmann_json::nlohmann_json
                          spdlog::spdlog
                          )
else()
    target_link_libraries(traders
                          PRIVATE
                          protocols
                          common
                          databases
                          strategies
                          Boost::system
                          OpenSSL::SSL
                          nlohmann_json::nlohmann_json
                          spdlog::spdlog
                          )
endif()
