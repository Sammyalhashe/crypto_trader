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
, d_config(config)
{
}

CoinbaseWebSocketClientAsync::~CoinbaseWebSocketClientAsync() {}

// PRIVATE MANIPULATORS

void CoinbaseWebSocketClientAsync::on_resolve(
    beast::error_code ec, tcp::resolver::results_type results)
{
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
            spdlog::info("req: {}", ss.str());
        }));

    d_ws.async_handshake(
        d_config.d_host,
        "/",
        beast::bind_front_handler(&CoinbaseWebSocketClientAsync::on_handshake,
                                  shared_from_this()));
}

void CoinbaseWebSocketClientAsync::on_handshake(beast::error_code ec)
{
    if (ec) {
        return fail(ec, "handshake");
    }

    d_ws.async_write(
        net::buffer(d_config.d_text.dump()),
        beast::bind_handler(&CoinbaseWebSocketClientAsync::on_write,
                            shared_from_this()));
}

void CoinbaseWebSocketClientAsync::on_write(beast::error_code ec,
                                            std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        return fail(ec, "write");
    }
}

void CoinbaseWebSocketClientAsync::on_read(beast::error_code ec,
                                           std::size_t       bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        return fail(ec, "read");
    }

    if (d_config.d_listenCb) {
        auto        res = d_buffer.data();
        std::string res_string(boost::asio::buffers_begin(res),
                               boost::asio::buffers_end(res));

        d_config.d_listenCb(res_string);
    }
}

void CoinbaseWebSocketClientAsync::do_read()
{
    if (d_ws.is_open()) {
        d_ws.async_read(
            d_buffer,
            beast::bind_front_handler(&CoinbaseWebSocketClientAsync::on_read,
                                      shared_from_this()));
    }
}

// WebsocketClient
void CoinbaseWebSocketClientAsync::open()
{
    d_resolver.async_resolve(
        d_config.d_host,
        s_port,
        beast::bind_front_handler(&CoinbaseWebSocketClientAsync::on_resolve,
                                  shared_from_this()));
}

void CoinbaseWebSocketClientAsync::close()
{
    spdlog::info("calling close!!!");
}

void CoinbaseWebSocketClientAsync::listen() { d_ioc.run(); }

bool CoinbaseWebSocketClientAsync::send_message() { return true; }

} // namespace adaptors
} // namespace crypto_trader
