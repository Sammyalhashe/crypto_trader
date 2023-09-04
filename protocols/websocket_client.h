#ifndef INCLUDED_WEBSOCKET_CLIENT
#define INCLUDED_WEBSOCKET_CLIENT

namespace crypto_trader {
namespace protocols {

class WebsocketClient {
    
public:
    // CREATORS
    WebsocketClient();
    virtual ~WebsocketClient() = 0;

    // PUBLIC MANIPULATORS
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool is_open() = 0;
    virtual bool send_message() = 0;
    virtual void listen() = 0;
}; // WebsocketClient

} // protocols
} // crypto_trader

#endif // INCLUDED_WEBSOCKET_CLIENT
