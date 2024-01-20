set(ADAPTORS_H coinbase_websocket_client.h coinbase_websocket_client_async.h database_websocket_client.h)
set(ADAPTORS_SRC coinbase_websocket_client.cpp coinbase_websocket_client_async.cpp database_websocket_client.cpp)

add_library(adaptors STATIC ${ADAPTORS_H} ${ADAPTORS_SRC})

# TODO: See how you can link two subdirectories
# together so you don't have to link everything
# together in the main executable.
target_link_libraries(adaptors
                      PRIVATE
                      protocols
                      databases
                      Boost::system
                      openssl::openssl
                      nlohmann_json::nlohmann_json
                      spdlog::spdlog)
