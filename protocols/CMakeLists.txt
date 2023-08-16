set(PROTOCOLS_H websocket_client.h)
set(PROTOCOLS_SRC websocket_client.cpp)

add_library(protocols STATIC ${PROTOCOLS_H} ${PROTOCOLS_SRC})
