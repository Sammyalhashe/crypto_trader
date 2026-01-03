set(STRATEGIES_H index.h simple.h hodl.h)
set(STRATEGIES_SRC simple.cpp hodl.cpp)

add_library(strategies STATIC ${STRATEGIES_H} ${STRATEGIES_SRC})


target_sources(strategies
  PUBLIC
    FILE_SET CXX_MODULES FILES
      risk_manager_v1.cppm
)

if(CONAN)
    target_link_libraries(strategies
                          PRIVATE
                          protocols
                          common
                          Boost::system
                          openssl::openssl
                          nlohmann_json::nlohmann_json
                          spdlog::spdlog
                          )
else()
    target_link_libraries(strategies
                          PRIVATE
                          protocols
                          common
                          Boost::system
                          OpenSSL::SSL
                          nlohmann_json::nlohmann_json
                          spdlog::spdlog
                          )
endif()
