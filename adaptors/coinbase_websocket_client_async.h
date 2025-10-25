#ifndef INCLUDED_COINBASE_WEBSOCKET_CLIENT_ASYNC
#define INCLUDED_COINBASE_WEBSOCKET_CLIENT_ASYNC

// See
// https://www.boost.org/doc/libs/develop/libs/beast/example/websocket/client/async-ssl/websocket_client_async_ssl.cpp
// for implementation details

#include "../protocols/websocket_client.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core/tcp_stream.hpp>
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

struct CoinbaseWebSocketClientAsyncConfig {
    // PUBLIC TYPES
    using ListenCb = std::function<void(const std::string&)>;
    // DATA
    std::string    d_host; // The host we are connecting to
    nlohmann::json d_text; // The initial message we send to the host
    ListenCb d_listenCb; // Function that is called when a websocket message is
                         // received.
    // Shared atomic state regarding if the program should still be running
    // or not.
    std::shared_ptr<std::atomic_bool> d_isRunning;

    // CREATORS
    CoinbaseWebSocketClientAsyncConfig(
        const std::string&                       host,
        const nlohmann::json&                    text,
        const ListenCb&                          listenCb,
        const std::shared_ptr<std::atomic_bool>& isRunning)
    : d_host(host)
    , d_text(text)
    , d_listenCb(listenCb)
    , d_isRunning(isRunning)
    {
    }
};

// class CoinbaseWebSocketClient
class CoinbaseWebSocketClientAsync
: public protocols::WebsocketClient,
  public std::enable_shared_from_this<CoinbaseWebSocketClientAsync> {

  private:
    // PRIVATE TYPES
    using WebsocketStream =
        websocket::stream<beast::ssl_stream<beast::tcp_stream>>;

    // PRIVATE DATA
    // These perform our I/O
    net::io_context d_ioc; // The io_context is required for all I/O
    ssl::context    d_ctx; // The ssl context is required, and holds all certs
    tcp::resolver   d_resolver;
    WebsocketStream d_ws;
    // Buffer to hold received messages
    beast::flat_buffer d_buffer;
    // The config for this object
    CoinbaseWebSocketClientAsyncConfig d_config;

    // PRIVATE STATIC DATA
    static std::string s_port;

    // DELETED METHODS
    CoinbaseWebSocketClientAsync(const CoinbaseWebSocketClientAsync& other) =
        delete;

  public:
    // CREATORS
    CoinbaseWebSocketClientAsync(
        const CoinbaseWebSocketClientAsyncConfig& config);
    ~CoinbaseWebSocketClientAsync();

    // MANIPULATORS
    void listen() override;

  private:
    // PRIVATE MANIPULATORS
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code                          ec,
                    tcp::resolver::results_type::endpoint_type ep);
    void on_ssl_handshake(beast::error_code ec);
    void on_handshake(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void do_read();

    // WebsocketClient
    void open() override;
    void close() override;
    bool is_open() override;
    bool send_message() override;
};

} // namespace adaptors
} // namespace crypto_trader

#endif // INCLUDED_COINBASE_WEBSOCKET_CLIENT_ASYNC
