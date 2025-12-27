set(ADAPTORS_H coinbase_websocket_client.h coinbase_websocket_client_async.h)
set(ADAPTORS_SRC coinbase_websocket_client.cpp coinbase_websocket_client_async.cpp)

add_library(adaptors STATIC ${ADAPTORS_H} ${ADAPTORS_SRC})

target_precompile_headers(adaptors
                          PRIVATE
                          <boost/asio.hpp>
                          <spdlog/spdlog.h>
                          <boost/beast.hpp>
                          <boost/bind.hpp>)

# TODO: See how you can link two subdirectories
# together so you don't have to link everything
# together in the main executable.
if(CONAN)
    target_link_libraries(adaptors
                          PRIVATE
                          protocols
                          databases
                          Boost::system
                          openssl::openssl
                          nlohmann_json::nlohmann_json
                          spdlog::spdlog)
else()
    target_link_libraries(adaptors
                          PRIVATE
                          protocols
                          databases
                          Boost::system
                          OpenSSL::SSL
                          nlohmann_json::nlohmann_json
                          spdlog::spdlog)
endif()
