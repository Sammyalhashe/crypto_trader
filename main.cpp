#include "traders/CoinbaseTrader.h"
#include "zig/zigmath.h"

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

import fileutils_module;
import json_module;

int main(int argc, char *argv[])
{
    spdlog::info("Result from zig: {}", add(1, 2));
    using namespace crypto_trader;

    SignalContext context;
    context.d_isRunning  = std::make_shared<std::atomic_bool>(true);
    *context.d_isRunning = true;

#ifdef __linux__
    const char *trapFileName = "/tmp/crypto_trader";
    int         inotify_fd   = fileutils_module::createTrapFile(trapFileName);
    fileutils_module::MonitorConfig monitorConfig{
        .d_inotify_fd   = inotify_fd,
        .d_trapFilePath = trapFileName,
        .d_isRunning    = context.d_isRunning};

    boost::thread trapFileWatchThread{
        boost::bind(&fileutils_module::monitorTrapFile, monitorConfig)};
#endif // __linux__

#if TARGET_OS_MAC
    fileutils_module::MonitorConfig monitorConfig{
        .d_trapFilePath = "/var/tmp/crypto_trader/coinbase_trader_data",
        .d_isRunning    = context.d_isRunning};
    boost::thread fsRunLoopThread{
        boost::bind(&fileutils_module::createEventStream, monitorConfig)};
#endif // TARGET_OS_MAC

    nlohmann::json jsonFileContents;
    spdlog::info("argc {}", argc);
    if (argc > 1) {
        std::stringstream ss;
        ss << argv[1];
        spdlog::info("passed in: {}", ss.str());

        int rc = fileutils_module::readJsonFile(&jsonFileContents, argv[1]);
        assert(0 == rc);
    }

    spdlog::info("starting crypto_trader");

    std::vector<std::unique_ptr<protocols::Trader>> traders;
    unsigned int                                    numTraders = 0;
    if (jsonFileContents.contains("traders")) {
        for (const auto& trader : jsonFileContents["traders"].items()) {
            std::stringstream ss;
            ss << trader.key() << " " << trader.value().dump(4);
            spdlog::info("configuring trader {}", ss.str());
            for (const auto& traderConfig : trader.value().items()) {
                if (traderConfig.key() == "coinbaseTrader") {
                    auto coinbaseTraderJson = traderConfig.value();
                    traders::CoinbaseTraderConfig coinbaseTraderConfig(
                        context.d_isRunning);
                    coinbaseTraderConfig.setUrl(json_module::value_or(
                        coinbaseTraderJson,
                        "url",
                        "wss://ws-feed-public.sandbox.exchange.coinbase.com"));
                    auto products = json_module::value_or(
                        coinbaseTraderJson, "products", "[\"ETH-USD\"]"_json);

                    std::vector<std::string> traderProducts;
                    for (const auto& product : products) {
                        traderProducts.push_back(product);
                    }

                    coinbaseTraderConfig.setProducts(traderProducts);

                    auto channelsJson = json_module::value_or(
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
                    coinbaseTraderConfig.setNumThreads(json_module::value_or(
                        coinbaseTraderJson, "numThreads", 1));
                    if (coinbaseTraderJson.contains("strategy")) {
                        const auto strategyJson =
                            coinbaseTraderJson["strategy"];
                        const auto& type = json_module::value_or(
                            strategyJson, "type", "hodl");
                        std::stringstream ss;
                        ss << type;
                        spdlog::info("strat: {}", ss.str());
                        if (type.get<std::string>() == "hodl") {
                            coinbaseTraderConfig.setStrategy(
                                strategies::e_HODL);
                        }
                        else {
                            coinbaseTraderConfig.setStrategy(
                                strategies::e_NONE);
                        }
                        coinbaseTraderConfig.setStrategyConfig(
                            json_module::value_or(
                                strategyJson, "config", "{}"_json));
                    }

                    auto clientType = json_module::value_or(
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

    fileutils_module::removeTrapFile(monitorConfig);
#endif // __linux__

#if TARGET_OS_MAC
    fsRunLoopThread.join();
#endif // TARGET_OS_MAC

    return 0;
}
