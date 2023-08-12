#ifndef INCLUDED_COINBASE_WEBSOCKET_CLIENT
#define INCLUDED_COINBASE_WEBSOCKET_CLIENT

#include "../protocols/websocket_client.h"

namespace crypto_trader {
namespace adaptors {

// class CoinbaseWebSocketClient
class CoinbaseWebSocketClient : private protocols::WebsocketClient {
    
public:
    // CREATORS
    CoinbaseWebSocketClient();
    ~CoinbaseWebSocketClient();

    // MANIPULATORS

private:
    // PRIVATE MANIPULATORS

    // WebsocketClient
    bool open() override;
    void close() override;
    bool is_open() override;
    bool send_message() override;
};

} // adaptors
} // crypto_trader

#endif // INCLUDED_COINBASE_WEBSOCKET_CLIENT
