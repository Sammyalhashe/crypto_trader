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
#include <iostream>
#include <signal.h>
#include <stdio.h>

namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;

#define DO_ONCE(var, expr) \
{ \
    if (!var) { \
        std::cout << "called!" << std::endl; \
        expr; \
    } \
}

static std::function<void(void)> s_cleaner;

struct Context {
    std::atomic<bool> d_isRunning = true;
    crypto_trader::protocols::WebsocketClient *d_client;
};

void cleaner(Context* context) 
{
    std::cout << "cleaner is called!" << std::endl;
    if (context) {
        context->d_isRunning = false;
        if (context->d_client) {
            context->d_client->close();
        }
    }
}

void INTHandler(int);

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

    Context context;
    context.d_client = &client;
    DO_ONCE(s_cleaner, {
        s_cleaner = std::bind(&cleaner, &context);
    });

    // setup ctrl-c handler
    signal(SIGINT, INTHandler);

    client.listen();

    return 0;
}

void INTHandler(int sig) {
    // tells the kernel to ignore the signal so we can handle ourselves
    signal(sig, SIG_IGN);
    s_cleaner();
}
