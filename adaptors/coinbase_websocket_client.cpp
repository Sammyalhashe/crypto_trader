#include "coinbase_websocket_client.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <iostream>
#include <string>

namespace crypto_trader {
namespace adaptors {

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// CoinbaseWebSocketClient
std::string CoinbaseWebSocketClient::s_port = "443";

// CREATORS
CoinbaseWebSocketClient::CoinbaseWebSocketClient(
                                   const CoinbaseWebSocketClientConfig& config)
: d_resolver(net::make_strand(config.d_ioc))
, d_ws(net::make_strand(config.d_ioc), config.d_ctx)
, d_config(config)
{
    open();
}

CoinbaseWebSocketClient::~CoinbaseWebSocketClient()
{
    if (d_ws.is_open()) {
        close();
    }
}


// MANIPULATORS

// PRIVATE MANIPULATORS

// WebsocketClient
bool CoinbaseWebSocketClient::open()
{

    // Look up the domain name
    const auto results = d_resolver.resolve(d_config.d_host, s_port);

    // Make the connection on the IP address we get from a lookup
    auto ep = net::connect(boost::beast::get_lowest_layer(d_ws), results);

    // Set SNI Hostname, which many hosts need to handshake successfully
    if(!SSL_set_tlsext_host_name(d_ws.next_layer().native_handle(),
                                 d_config.d_host.c_str()))
    {
        throw beast::system_error(
          beast::error_code(
              static_cast<int>(::ERR_get_error()),
              net::error::get_ssl_category()),
          "Failed to set SNI Hostname");
    }

    // perform ssl handshake
    spdlog::info("performing ssl handshake");
    d_ws.next_layer().handshake(ssl::stream_base::client);

    // Set a decorator to change the user-agent of the request.
    d_ws.set_option(
        websocket::stream_base::decorator([](websocket::request_type& req) {
          req.set(http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING) + " crypto_trader");
          std::stringstream ss;
          ss << req;
          spdlog::info("req: {}", ss.str());
        }));

    // perform the websocket handshake
    d_ws.handshake(d_config.d_host + ":" + std::to_string(ep.port()), "/");

    // Send the message
    spdlog::info("Writing the request:\n{}\nto coinbase",
                 d_config.d_text.dump(4));
    d_ws.write(net::buffer(d_config.d_text.dump()));

    // read a message into the buffer
    beast::flat_buffer buffer;
    d_ws.read(buffer);

    // write message received to stdout
    std::stringstream ss;
    ss << beast::make_printable(buffer.data());
    spdlog::info("message received: {}", ss.str());

    return true;
}

void CoinbaseWebSocketClient::close()
{
    // Close the WebSocket connection
    spdlog::info("calling close!!!");
    d_ws.close(websocket::close_code::normal);
}

bool CoinbaseWebSocketClient::is_open()
{
    return true;
}

bool CoinbaseWebSocketClient::send_message()
{
    return true;
}

void CoinbaseWebSocketClient::listen()
{
    while (d_ws.is_open() && *d_config.d_isRunning) {
        // read a message into the buffer
        beast::flat_buffer buffer;
        d_ws.read(buffer);

        // write message received to stdout
        std::stringstream ss;
        ss << beast::make_printable(buffer.data());
        spdlog::info("message received: {}", ss.str());

    }

    if (d_ws.is_open()) {
        close();
    }
}

} // adaptors
} // crypto_trader
