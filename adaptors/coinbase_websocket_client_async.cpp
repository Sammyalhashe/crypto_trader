#include "coinbase_websocket_client_async.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind/bind.hpp>

#include <spdlog/spdlog.h>
#include <thread>

namespace crypto_trader {
namespace adaptors {

namespace beast     = boost::beast; // from <boost/beast.hpp>
namespace http      = beast::http;  // from <boost/beast/http.hpp>
namespace net       = boost::asio;  // from <boost/asio.hpp>
namespace ssl       = boost::asio::ssl;
namespace websocket = beast::websocket;     // from <boost/beast/websocket.hpp>
using tcp           = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace {

void fail(beast::error_code ec, const char *what)
{
    std::stringstream ss;
    ss << ec;
    spdlog::error("Failed during {} with error code: {}", what, ss.str());
}

}; // unnamed namespace

// CoinbaseWebSocketClientAsync
std::string CoinbaseWebSocketClientAsync::s_port = "443";

// CREATORS
CoinbaseWebSocketClientAsync::CoinbaseWebSocketClientAsync(
    const CoinbaseWebSocketClientAsyncConfig& config)
: d_ioc()
, d_ctx(ssl::context::tlsv12_client)
, d_resolver(net::make_strand(d_ioc))
, d_ws(net::make_strand(d_ioc), d_ctx)
, d_reconnectionAttempts(0)
, d_config(config)
{
}

CoinbaseWebSocketClientAsync::~CoinbaseWebSocketClientAsync() { close(); }

// PRIVATE MANIPULATORS

void CoinbaseWebSocketClientAsync::on_resolve(
    beast::error_code ec, tcp::resolver::results_type results)
{
    spdlog::debug("CoinbaseWebSocketClientAsync::on_resolve");
    if (ec) {
        return fail(ec, "resolve");
    }

    // Set a timeout on the operation
    // TODO: config val?
    beast::get_lowest_layer(d_ws).expires_after(std::chrono::seconds(30));

    beast::get_lowest_layer(d_ws).async_connect(
        results,
        beast::bind_front_handler(&CoinbaseWebSocketClientAsync::on_connect,
                                  shared_from_this()));
}

void CoinbaseWebSocketClientAsync::on_connect(
    beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
    spdlog::debug("CoinbaseWebSocketClientAsync::on_connect");
    if (ec) {
        return fail(ec, "connect");
    }

    // Set a timeout on the operation
    // TODO: config val?
    beast::get_lowest_layer(d_ws).expires_after(std::chrono::seconds(30));

    // Set SNI Hostname, which many hosts need to handshake successfully
    if (!SSL_set_tlsext_host_name(d_ws.next_layer().native_handle(),
                                  d_config.d_host.c_str()))
    {
        beast::error_code ec =
            beast::error_code(static_cast<int>(::ERR_get_error()),
                              net::error::get_ssl_category());
        return fail(ec, "connect");
    }

    d_config.d_host += ":" + std::to_string(ep.port());

    // Perform the SSL handshake
    d_ws.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &CoinbaseWebSocketClientAsync::on_ssl_handshake,
            shared_from_this()));
}

void CoinbaseWebSocketClientAsync::on_ssl_handshake(beast::error_code ec)
{
    spdlog::debug("CoinbaseWebSocketClientAsync::on_ssl_handshake");
    if (ec) {
        return fail(ec, "ssl_handshake");
    }

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(d_ws).expires_never();

    // Set suggested timeout settings for the websocket
    d_ws.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the user-agent of the request.
    d_ws.set_option(
        websocket::stream_base::decorator([](websocket::request_type& req) {
            req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " crypto_trader");
            std::stringstream ss;
            ss << req;
            SPDLOG_INFO("req: {}", ss.str());
        }));

    d_ws.async_handshake(
        d_config.d_host,
        "/",
        beast::bind_front_handler(&CoinbaseWebSocketClientAsync::on_handshake,
                                  shared_from_this()));
}

void CoinbaseWebSocketClientAsync::on_handshake(beast::error_code ec)
{
    spdlog::debug("CoinbaseWebSocketClientAsync::on_handshake");
    if (ec) {
        return fail(ec, "handshake");
    }

    SPDLOG_INFO("Writing the request:\n{}\nto coinbase",
                 d_config.d_text.dump(4));
    d_ws.async_write(
        net::buffer(d_config.d_text.dump()),
        beast::bind_front_handler(&CoinbaseWebSocketClientAsync::on_write,
                                  shared_from_this()));
}

void CoinbaseWebSocketClientAsync::on_write(beast::error_code ec,
                                            std::size_t bytes_transferred)
{
    spdlog::debug("CoinbaseWebSocketClientAsync::on_write");
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        return fail(ec, "write");
    }

    do_read();
}

void CoinbaseWebSocketClientAsync::on_read(beast::error_code ec,
                                           std::size_t       bytes_transferred)
{
    spdlog::debug("CoinbaseWebSocketClientAsync::on_read");
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        if (d_reconnectionAttempts < d_config.d_maxReconnectAttempts) {
            spdlog::warn("Connection lost, reconnecting (attempt {})",
                         ++d_reconnectionAttempts);

            // Exponential backoff
            std::this_thread::sleep_for(d_config.d_reconnectDelay *
                                        (1 << (d_reconnectionAttempts - 1)));

            open();
            return;
        }
        return fail(ec, "Max reconnects exceeded");
    }

    // reset on success
    d_reconnectionAttempts = 0;

    if (d_config.d_listenCb) {
        auto        res = d_buffer.data();
        std::string res_string(boost::asio::buffers_begin(res),
                               boost::asio::buffers_end(res));

        std::stringstream ss;
        ss << beast::make_printable(d_buffer.data());
        spdlog::debug("message received: {}", ss.str());

        d_config.d_listenCb(res_string);

        // Clear the buffer
        d_buffer.consume(d_buffer.size());
    }

    do_read();
}

void CoinbaseWebSocketClientAsync::do_read()
{
    if (d_ws.is_open() && *d_config.d_isRunning) {
        d_ws.async_read(
            d_buffer,
            beast::bind_front_handler(&CoinbaseWebSocketClientAsync::on_read,
                                      shared_from_this()));
    }
}

// WebsocketClient
void CoinbaseWebSocketClientAsync::open()
{
    SPDLOG_INFO("CoinbaseWebSocketClientAsync::open");
    d_resolver.async_resolve(
        d_config.d_host,
        s_port,
        beast::bind_front_handler(&CoinbaseWebSocketClientAsync::on_resolve,
                                  shared_from_this()));
}

void CoinbaseWebSocketClientAsync::close()
{
    SPDLOG_INFO("calling close!!!");
    d_ws.close(websocket::close_code::normal);
    d_ioc.stop();
}

void CoinbaseWebSocketClientAsync::listen()
{
    spdlog::debug("CoinbaseWebSocketClientAsync::listen");
    open();

    // blocks until websocket connection is closed.
    d_ioc.run();
}

bool CoinbaseWebSocketClientAsync::is_open() { return true; }

bool CoinbaseWebSocketClientAsync::send_message() { return true; }

} // namespace adaptors
} // namespace crypto_trader
