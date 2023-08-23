#include "adaptors/coinbase_websocket_client.h"
#include "protocols/websocket_client.h"
#include "strategies/simple.h"
#include "testlib/test.h"

#include <boost/beast/ssl.hpp>

#include <nlohmann/json.hpp>

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <signal.h>
#include <stdio.h>

namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;

#define DO_ONCE(var, expr) \
{ \
    if (!var) { \
        expr; \
    } \
}

static std::function<void(void)> s_cleaner;

struct SignalContext {
    std::shared_ptr<std::atomic<bool>> d_isRunning;
};

int main() {
    SignalContext context;
    context.d_isRunning = std::make_shared<std::atomic_bool>(true);
    *context.d_isRunning = true;

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
                                                   json,
                                                   context.d_isRunning);
    adaptors::CoinbaseWebSocketClient client(config);


    client.listen();

    return 0;
}
