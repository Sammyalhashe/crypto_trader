#include "adaptors/coinbase_websocket_client.h"
#include "strategies/simple.h"
#include "testlib/test.h"

#include <boost/beast/ssl.hpp>

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <cstring>
#include <stdio.h>

namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;

int main() {
    using namespace crypto_trader;
    strategies::simpleStrategy();

    static const std::string& ws_feed_host =
                                  "ws-feed.exchange.coinbase.com";
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
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    adaptors::CoinbaseWebSocketClientConfig config(ioc,
                                                   ctx,
                                                   ws_feed_host,
                                                   json);
    adaptors::CoinbaseWebSocketClient client(config);

    return 0;
}
