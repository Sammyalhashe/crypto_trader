#include "coinbase_websocket_client.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

#include <nlohmann/json.hpp>

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

// CREATORS
CoinbaseWebSocketClient::CoinbaseWebSocketClient()
{
    open();
}

CoinbaseWebSocketClient::~CoinbaseWebSocketClient()
{}


// MANIPULATORS

// PRIVATE MANIPULATORS

// WebsocketClient
bool CoinbaseWebSocketClient::open()
{
    // TODO: Move these to constants file
    static const std::string& ws_feed_host =
                                  "ws-feed-public.sandbox.exchange.coinbase.com";
    static const std::string& port = "443";

    static const nlohmann::json& json =
        "{"
            "\"type\": \"subscribe\","
            "\"product_ids\": ["
                "\"ETH-EUR\""
            "],"
            "\"channels\": ["
                "\"heartbeat\","
                "{"
                    "\"name\": \"ticker\","
                    "\"product_ids\": [\"ETH-EUR\"]"
                "}"
            "]"
          "}"_json
        ;

    // The io_context is required for all I/O
    net::io_context ioc;

    // The ssl context is required, and holds all certs
    ssl::context ctx{ssl::context::tlsv12_client};

    // These perform our I/O
    tcp::resolver resolver{ioc};
    websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

    // Look up the domain name
    const auto results = resolver.resolve(ws_feed_host, port);

    // Make the connection on the IP address we get from a lookup
    auto ep = net::connect(boost::beast::get_lowest_layer(ws), results);

    // Set SNI Hostname, which many hosts need to handshake successfully
    if(!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), ws_feed_host.c_str()))
    {
        throw beast::system_error(
          beast::error_code(
              static_cast<int>(::ERR_get_error()),
              net::error::get_ssl_category()),
          "Failed to set SNI Hostname");
    }

    // perform ssl handshake
    std::cout << "performing ssl handshake" << std::endl;
    ws.next_layer().handshake(ssl::stream_base::client);

    // Set a decorator to change the user-agent of the request.
    ws.set_option(
        websocket::stream_base::decorator([](websocket::request_type& req) {
          req.set(http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING) + " crypto_trader");
          std::cout << "req: " << req << std::endl;
        }));

    // perform the websocket handshake
    ws.handshake(ws_feed_host + ":" + std::to_string(ep.port()), "/");

    // Send the message
    std::cout << "Writing the request:\n"
              << json.dump(4) // NOTE: pretty printing
              << "\nto coinbase"
              << std::endl;
    ws.write(net::buffer(json.dump()));

    // read a message into the buffer
    beast::flat_buffer buffer;
    ws.read(buffer);

    // Close the WebSocket connection
    ws.close(websocket::close_code::normal);

    // write message received to stdout
    std::cout << beast::make_printable(buffer.data()) << std::endl;

    return true;
}

void CoinbaseWebSocketClient::close()
{

}

bool CoinbaseWebSocketClient::is_open()
{
    return true;
}

bool CoinbaseWebSocketClient::send_message()
{
    return true;
}

} // adaptors
} // crypto_trader
