#include "common/fileutils.h"
#include "common/jsonutils.h"
#include "protocols/trader.h"
#include "traders/CoinbaseTrader.h"
#include "zig/zigmath/zigmath.h"

#include <boost/asio/thread_pool.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/thread.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <signal.h>
#include <stdio.h>
#include <variant>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#define DO_ONCE(var, expr)                                                    \
    {                                                                         \
        if (!var) {                                                           \
            expr;                                                             \
        }                                                                     \
    }

static std::function<void(void)> s_cleaner;

struct SignalContext {
    std::shared_ptr<std::atomic<bool>> d_isRunning;
};

int main(int argc, char *argv[])
{
    spdlog::set_pattern("[source %s] [function %!] [line %#] %v");
    SPDLOG_INFO("Result from zig: {}", add(1, 2));
    // SPDLOG_INFO();
    using namespace crypto_trader;

    SignalContext context;
    context.d_isRunning  = std::make_shared<std::atomic_bool>(true);
    *context.d_isRunning = true;

#ifdef __linux__
    const char           *trapFileName = "/tmp/crypto_trader";
    int                   inotify_fd   = common::createTrapFile(trapFileName);
    common::MonitorConfig monitorConfig{.d_inotify_fd   = inotify_fd,
                                        .d_trapFilePath = trapFileName,
                                        .d_isRunning    = context.d_isRunning};

    boost::thread trapFileWatchThread{
        boost::bind(&common::monitorTrapFile, monitorConfig)};
#endif // __linux__

#if TARGET_OS_MAC
    common::MonitorConfig monitorConfig{
        .d_trapFilePath = "/var/tmp/crypto_trader/coinbase_trader_data",
        .d_isRunning    = context.d_isRunning};
    boost::thread fsRunLoopThread{
        boost::bind(&common::createEventStream, monitorConfig)};
#endif // TARGET_OS_MAC

    nlohmann::json jsonFileContents;
    SPDLOG_INFO("argc {}", argc);
    if (argc > 1) {
        std::stringstream ss;
        ss << argv[1];
        SPDLOG_INFO("passed in: {}", ss.str());

        int rc = common::readJsonFile(&jsonFileContents, argv[1]);
        assert(0 == rc);
    }

    SPDLOG_INFO("starting crypto_trader");

    std::vector<std::unique_ptr<protocols::Trader>> traders;
    unsigned int                                    numTraders = 0;

    bool paperTrading =
        common::value_or(jsonFileContents, "paperTrading", true);
    if (jsonFileContents.contains("traders")) {
        for (const auto& trader : jsonFileContents["traders"].items()) {
            std::stringstream ss;
            ss << trader.key() << " " << trader.value().dump(4);
            SPDLOG_INFO("configuring trader {}", ss.str());
            for (const auto& traderConfig : trader.value().items()) {
                if (traderConfig.key() == "coinbaseTrader") {
                    auto coinbaseTraderJson = traderConfig.value();
                    traders::CoinbaseTraderConfig coinbaseTraderConfig(
                        context.d_isRunning);
                    coinbaseTraderConfig.setPaperTrading(paperTrading);
                    coinbaseTraderConfig.setUrl(common::value_or(
                        coinbaseTraderJson,
                        "url",
                        "wss://ws-feed-public.sandbox.exchange.coinbase.com"));
                    auto products = common::value_or(
                        coinbaseTraderJson, "products", "[\"ETH-USD\"]"_json);

                    std::vector<std::string> traderProducts;
                    for (const auto& product : products) {
                        traderProducts.push_back(product);
                    }

                    coinbaseTraderConfig.setProducts(traderProducts);

                    auto channelsJson = common::value_or(
                        coinbaseTraderJson, "channels", "[]"_json);

                    std::vector<std::variant<
                        std::string,
                        traders::CoinbaseTraderConfig::ChannelDefinition>>
                        channels;
                    for (const auto& channel : channelsJson) {
                        if (channel.is_string()) {
                            channels.push_back(channel.get<std::string>());
                        }
                        else {
                            std::vector<std::string> channelProducts;
                            for (const auto& channelProduct :
                                 channel["products"].items())
                            {
                                channelProducts.push_back(
                                    channelProduct.value());
                            }

                            channels.emplace_back(
                                traders::CoinbaseTraderConfig::
                                    ChannelDefinition{
                                        .d_name     = channel["name"],
                                        .d_products = channelProducts});
                        }
                    }

                    coinbaseTraderConfig.setChannels(channels);
                    coinbaseTraderConfig.setNumThreads(
                        common::value_or(coinbaseTraderJson, "numThreads", 1));
                    if (coinbaseTraderJson.contains("strategy")) {
                        const auto strategyJson =
                            coinbaseTraderJson["strategy"];
                        const auto& type =
                            common::value_or(strategyJson, "type", "hodl");
                        std::stringstream ss;
                        ss << type;
                        SPDLOG_INFO("strat: {}", ss.str());
                        if (type.get<std::string>() == "hodl") {
                            coinbaseTraderConfig.setStrategy(
                                strategies::e_HODL);
                            coinbaseTraderConfig.setStrategyConfig(
                                common::value_or(
                                    strategyJson, "config", "{}"_json));
                        }
                        else {
                            coinbaseTraderConfig.setStrategy(
                                strategies::e_NONE);
                            coinbaseTraderConfig.setStrategyConfig(
                                common::value_or(
                                    strategyJson, "config", "{}"_json));
                        }
                    }

                    auto clientType = common::value_or(
                        coinbaseTraderJson, "clientType", "SYNC");

                    if (clientType == "SYNC") {
                        coinbaseTraderConfig.setClientType(
                            traders::CoinbaseTraderConfig::ClientType::SYNC);
                    }
                    else if (clientType == "ASYNC") {
                        coinbaseTraderConfig.setClientType(
                            traders::CoinbaseTraderConfig::ClientType::ASYNC);
                    }
                    else {
                        spdlog::error("Recieved invalid client type for "
                                      "coinbase trader, "
                                      "defaulting to SYNC");
                        coinbaseTraderConfig.setClientType(
                            traders::CoinbaseTraderConfig::ClientType::SYNC);
                    }

                    traders.push_back(
                        std::make_unique<traders::CoinbaseTrader>(
                            coinbaseTraderConfig));
                }

                ++numTraders;
            }
        }
    }

    boost::asio::thread_pool running_traders(numTraders);

    for (const auto& trader : traders) {
        boost::asio::post(running_traders, [&trader]() { trader->start(); });
    }

    running_traders.join();

#ifdef __linux__
    trapFileWatchThread.join();

    common::removeTrapFile(monitorConfig);
#endif // __linux__

#if TARGET_OS_MAC
    fsRunLoopThread.join();
#endif // TARGET_OS_MAC

    return 0;
}
