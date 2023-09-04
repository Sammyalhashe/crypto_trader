#include "traders/CoinbaseTrader.h"

#include <boost/beast/ssl.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <signal.h>
#include <stdio.h>

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
    spdlog::info("starting crypto_trader");
    SignalContext context;
    context.d_isRunning = std::make_shared<std::atomic_bool>(true);
    *context.d_isRunning = true;

    using namespace crypto_trader;

    traders::CoinbaseTraderConfig coinbaseTraderConfig(context.d_isRunning);
    coinbaseTraderConfig.setUrl("ws-feed.exchange.coinbase.com");
    coinbaseTraderConfig.setProducts({"ETH-USD"});
    coinbaseTraderConfig.setChannels(
            {
            "heartbeat",
            traders::CoinbaseTraderConfig::ChannelDefinition{
                                                   .d_name     = "ticker",
                                                   .d_products = {"ETH-USD"}}
            });

    traders::CoinbaseTrader coinbaseTrader(coinbaseTraderConfig);

    coinbaseTrader.start();

    return 0;
}
