#ifndef INCLUDED_COINBASE_WEBSOCKET_CLIENT
#define INCLUDED_COINBASE_WEBSOCKET_CLIENT

#include "../protocols/websocket_client.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <nlohmann/json.hpp>

#include <atomic>
#include <string>

namespace crypto_trader {
namespace adaptors {

namespace beast     = boost::beast;     // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace ssl       = boost::asio::ssl;
namespace net       = boost::asio;          // from <boost/asio.hpp>
using tcp           = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

struct CoinbaseWebSocketClientConfig {
    // PUBLIC TYPES
    using ListenCb = std::function<void(const std::string&)>;
    // DATA
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
        const std::string                      & host,
        const nlohmann::json                   & text,
        const ListenCb                         & listenCb,
        const std::shared_ptr<std::atomic_bool>& isRunning)
    : d_host(host)
    , d_text(text)
    , d_listenCb(listenCb)
    , d_isRunning(isRunning)
    {
    }
};

// class CoinbaseWebSocketClient
class CoinbaseWebSocketClient : public protocols::WebsocketClient {

  private:
    // PRIVATE TYPES
    using WebsocketStream = websocket::stream<beast::ssl_stream<tcp::socket>>;

    // PRIVATE DATA
    // These perform our I/O
    net::io_context d_ioc; // The io_context is required for all I/O
    ssl::context    d_ctx; // The ssl context is required, and holds all certs
    tcp::resolver   d_resolver;
    WebsocketStream           d_ws;     // The websocket
    CoinbaseWebSocketClientConfig d_config; // The config for this object

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

} // namespace adaptors
} // namespace crypto_trader

#endif // INCLUDED_COINBASE_WEBSOCKET_CLIENT
