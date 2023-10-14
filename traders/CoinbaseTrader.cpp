#include "CoinbaseTrader.h"

#include "../adaptors/coinbase_websocket_client.h"
#include "../common/jsonutils.h"
#include "../protocols/websocket_client.h"
#include "../strategies/hodl.h"
#include "../strategies/index.h"

#include <boost/beast/ssl.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace crypto_trader {
namespace traders {

namespace net = boost::asio; // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;

namespace {

void buildCoinbaseWebsocketMessage(nlohmann::json             *message,
                                   const std::string         & type,
                                   const CoinbaseTraderConfig& config)
{
    std::string result;
    result += "{"
              "\"type\": \"" +
              type +
              "\","
              "\"product_ids\": [";

    unsigned int idx = 0;
    for (const auto& product : config.products()) {
        result += "\"" + product + "\"";
        if (config.products().size() - 1 > idx) {
            result += ",";
        }
        ++idx;
    }
    result += "],"
              "\"channels\": [";
    idx = 0;
    if (config.channels().has_value()) {
        for (const auto& channel : config.channels().value()) {
            if (std::holds_alternative<std::string>(channel)) {
                result += "\"" + std::get<std::string>(channel) + "\"";
            }
            else if (std::holds_alternative<
                         CoinbaseTraderConfig::ChannelDefinition>(channel))
            {
                const auto channelDef =
                    std::get<CoinbaseTraderConfig::ChannelDefinition>(channel);
                result += "{"
                          "\"name\": \"" +
                          channelDef.d_name +
                          "\","
                          "\"product_ids\": [";
                unsigned int idx2 = 0;
                for (const auto& productid : channelDef.d_products) {
                    result += "\"" + productid + "\"";
                    if (channelDef.d_products.size() - 1 > idx2) {
                        result += ",";
                    }
                    ++idx2;
                }
                result += "]"
                          "}";
            }
            else {
                ++idx;
                continue;
            }
            if (config.channels().value().size() - 1 > idx) {
                result += ",";
            }
            ++idx;
        }
    }
    result += "]"
              "}";

    try {
        *message = nlohmann::json::parse(result);
    }
    catch (nlohmann::json::parse_error& e) {
        std::cerr << e.what() << '\n';
    }
}

} // unnamed namespace

// class CoinbaseTraderConfig
CoinbaseTraderConfig::CoinbaseTraderConfig(
    const std::shared_ptr<std::atomic_bool>& isRunning)
: d_channels()
, d_products()
, d_strategy(strategies::e_NONE)
, d_strategyConfig()
, d_url()
, d_numThreads(1)
, d_isRunning(isRunning)
{
}

// class CoinbaseTrader

// CREATORS
CoinbaseTrader::CoinbaseTrader(const CoinbaseTraderConfig& config)
: d_webSocketClient()
, d_strategy()
, d_threadPool(config.numThreads())
, d_isStopped(true)
, d_mutex()
, d_config(config)
{
    switch (d_config.strategy()) {
    case strategies::TradingStrategy::e_HODL: {
        nlohmann::json result;
        buildCoinbaseWebsocketMessage(&result, "subscribe", d_config);
        spdlog::info("built result: {}", result.dump(4));
        net::io_context ioc;
        ssl::context    ctx{ssl::context::tlsv12_client};
        adaptors::CoinbaseWebSocketClientConfig coinbaseWebSocketConfig(
            ioc,
            ctx,
            config.url(),
            result,
            std::bind(&CoinbaseTrader::listen, this, std::placeholders::_1),
            d_config.isRunning());
        d_webSocketClient =
            std::make_unique<adaptors::CoinbaseWebSocketClient>(
                coinbaseWebSocketConfig);

        const auto& hodlConfigJson = d_config.strategyConfig();
        strategies::HodlStrategyConfig::InitStrategy initStrat;
        const auto& initStratString = common::value_or(
            hodlConfigJson, "initStrategy", "buy_immediately");
        if (initStratString.get<std::string>() == "buy_immediately") {
            initStrat = strategies::HodlStrategyConfig::e_BUY_IMMEDIATELY;
        }
        else if (initStratString.get<std::string>() == "set_basis_price") {
            initStrat = strategies::HodlStrategyConfig::e_SET_BASIS_PRICE;
        }
        else {
            initStrat = strategies::HodlStrategyConfig::e_BUY_IMMEDIATELY;
        }

        strategies::HodlStrategyConfig hodlConfig;
        hodlConfig
            .setPercentUp(common::value_or(hodlConfigJson, "percentUp", 5))
            .setPercentDown(common::value_or(hodlConfigJson, "percentDown", 5))
            .setInitStrategy(initStrat)
            .setEmit(std::bind(
                &CoinbaseTrader::processAction, this, std::placeholders::_1));

        d_strategy = std::make_unique<strategies::HodlStrategy>(hodlConfig);
    } break;
    default: {
        std::stringstream ss;
        ss << d_config.strategy();
        spdlog::error("CoinbaseTrader was configured with "
                      "unknown trading strategy: {}",
                      ss.str());
        assert(false);
    } break;
    }
}

CoinbaseTrader::~CoinbaseTrader() { stop(); }

// PUBLIC MANIPULATORS

void CoinbaseTrader::start()
{
    if (!d_isStopped) {
        return;
    }

    // mark this trader as started.
    d_isStopped = false;

    // TODO: Warn if not set?
    if (d_webSocketClient) {
        d_webSocketClient->listen();
    }
}

void CoinbaseTrader::stop()
{
    if (d_isStopped) {
        return;
    }

    // mark this trader as stopped.
    d_isStopped = true;

    // Wait till all events are handled.
    // NOTE: If trader was never started, no events should be enqueued on so
    // it should just end.
    d_threadPool.join();
}

void CoinbaseTrader::processAction(const common::Action& action)
{
    if (d_isStopped) {
        return;
    }
    std::stringstream ss;
    ss << action.d_type;
    spdlog::info("Processing action: {}", ss.str());

    boost::asio::post(d_threadPool,
                      std::bind(&CoinbaseTrader::handleAction, this, action));
}

void CoinbaseTrader::handleAction(const common::Action& action)
{
    std::stringstream ss;
    ss << action.d_type;
    spdlog::info("Handling action: {}", ss.str());
}

void CoinbaseTrader::handleNewData(const std::string_view& buffer)
{
    std::lock_guard<std::mutex> guard(d_mutex); // LOCK
    if (!*d_config.isRunning()) {
        d_threadPool.stop();
        return;
    }
    if (d_strategy) {
        d_strategy->handleNewData(buffer);
    }
}

// protocols::Trader
void CoinbaseTrader::listen(const std::string_view& buffer)
{
    {
        std::lock_guard<std::mutex> guard(d_mutex); // LOCK
        if (!*d_config.isRunning()) {
            d_threadPool.stop();
            return;
        }
    }

    if (d_isStopped) {
        return;
    }

    std::string v(buffer);
    boost::asio::post(d_threadPool,
                      std::bind(&CoinbaseTrader::handleNewData, this, v));
}

} // namespace traders
} // namespace crypto_trader
