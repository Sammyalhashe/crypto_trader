#ifndef INCLUDED_COINBASE_WEBSOCKET_CLIENT
#define INCLUDED_COINBASE_WEBSOCKET_CLIENT

#include "../protocols/websocket_client.h"


#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <functional>
#include <nlohmann/json.hpp>

#include <atomic>
#include <string>

namespace crypto_trader {
namespace adaptors {

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

struct CoinbaseWebSocketClientConfig {
    // PUBLIC TYPES
    using ListenCb = std::function<void(const std::string&)>;
    // DATA
    // The io_context is required for all I/O
    net::io_context& d_ioc;
    // The ssl context is required, and holds all certs
    ssl::context& d_ctx;
    // The host we are connecting to
    std::string d_host;
    // The initial message we send to the host
    nlohmann::json d_text;
    // Function that is called when a websocket message is received.
    ListenCb d_listenCb;
    // Shared atomic state regarding if the program should still be running
    // or not.
    std::shared_ptr<std::atomic_bool> d_isRunning;

    
    // CREATORS
    CoinbaseWebSocketClientConfig(
                            net::io_context&                         ioc,
                            ssl::context&                            ctx,
                            const std::string&                       host,
                            const nlohmann::json&                    text,
                            const ListenCb&                          listenCb,
                            const std::shared_ptr<std::atomic_bool>& isRunning)
    : d_ioc(ioc)
    , d_ctx(ctx)
    , d_host(host)
    , d_text(text)
    , d_listenCb(listenCb)
    , d_isRunning(isRunning)
    {}
};

// class CoinbaseWebSocketClient
class CoinbaseWebSocketClient : public protocols::WebsocketClient {

private:
    // PRIVATE TYPES
    using WebsocketStream = websocket::stream<beast::ssl_stream<tcp::socket>>;

    // PRIVATE DATA
    // These perform our I/O
    tcp::resolver d_resolver;
    WebsocketStream d_ws;
    // The config for this object
    CoinbaseWebSocketClientConfig d_config;

    // PRIVATE STATIC DATA
    static std::string s_port;

    // DELETED METHODS
    CoinbaseWebSocketClient(const CoinbaseWebSocketClient& other) = delete;

public:
    // CREATORS
    CoinbaseWebSocketClient(const CoinbaseWebSocketClientConfig& config);
    ~CoinbaseWebSocketClient();

    // MANIPULATORS
    void listen() override;

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
