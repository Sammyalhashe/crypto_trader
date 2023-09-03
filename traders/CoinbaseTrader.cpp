#include "CoinbaseTrader.h"

#include "../adaptors/coinbase_websocket_client.h"
#include "../protocols/websocket_client.h"
#include "../strategies/index.h"
#include "../strategies/hodl.h"

#include <boost/beast/ssl.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace crypto_trader {
namespace traders {

namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;

namespace {

    void buildCoinbaseWebsocketMessage(nlohmann::json              *message,
                                       const std::string&           host,
                                       const std::string&           type,
                                       const CoinbaseTraderConfig&  config)
    {
        std::string result;
        result += "{";
        result += "\"type\": \"" + type + "\",";
        result += "\"product_ids\": [";
        unsigned int idx = 0;
        for (const auto& product : config.products()) {
            result += "\"" + product + "\"";
            if (config.products().size() - 1 > idx) {
                result += ",";
            }
            ++idx;
        }
        result += "],";
        result += "\"channels\": [";
        idx = 0;
        if (config.channels().has_value()) {
            for (const auto& channel : config.channels().value()) {
                if (std::holds_alternative<std::string>(channel)) {
                    result += "\"" + std::get<std::string>(channel) + "\"";
                }
                else if (std::holds_alternative<
                             CoinbaseTraderConfig::ChannelDefinition>(channel))
                {
                    const auto channelDef = std::get<
                             CoinbaseTraderConfig::ChannelDefinition>(channel);
                    result += "{";
                    result += "\"name\": \"" + channelDef.d_name + "\",";
                    result += "\"product_ids\": [";
                    unsigned int idx2 = 0;
                    for (const auto& productid : channelDef.d_products) {
                        result += "\"" + productid + "\"";
                        if (channelDef.d_products.size() - 1 > idx2) {
                            result += ",";
                        }
                        ++idx2;
                    }
                    result += "]";
                    result += "}";
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
        result += "]";
        result += "}";

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
, d_strategy(strategies::e_HODL)
, d_isRunning(isRunning)
{
}

// class CoinbaseTrader

// CREATORS
CoinbaseTrader::CoinbaseTrader(const CoinbaseTraderConfig& config)
: d_webSocketClient()
, d_config(config)
{
    switch (d_config.strategy()) {
        case strategies::Strategy::e_HODL: {
            nlohmann::json result;
            buildCoinbaseWebsocketMessage(&result,
                                          config.url(),
                                          "subscribe",
                                          d_config);
            spdlog::info("built result: {}", result.dump(4));
            net::io_context ioc;
            ssl::context ctx{ssl::context::tlsv12_client};
            adaptors::CoinbaseWebSocketClientConfig coinbaseWebSocketConfig(
                           ioc,
                           ctx,
                           config.url(),
                           result,
                           std::bind(&CoinbaseTrader::listen,
                                     this,
                                     std::placeholders::_1),
                           d_config.isRunning());
            d_webSocketClient = std::make_unique<
                   adaptors::CoinbaseWebSocketClient>(coinbaseWebSocketConfig);

            // TODO: Figure out if I should send this in
            // coinbaseWebSocketConfig.
            strategies::HodlStrategyConfig hodlConfig;
            hodlConfig
                .setPercentUp(5)
                .setPercentDown(5)
                .setInitStrategy(
                            strategies::HodlStrategyConfig::e_BUY_IMMEDIATELY);

            d_strategy = std::make_unique<strategies::HodlStrategy>(
                                                                   hodlConfig);
        } break;
        default: {
            // TODO: Probably default to HODL and log a warning...
        } break;
    }
}

CoinbaseTrader::~CoinbaseTrader()
{}

// PUBLIC MANIPULATORS

void CoinbaseTrader::start()
{
    // TODO: Warn if not set?
    if (d_webSocketClient) {
        d_webSocketClient->listen();
    }
}

void CoinbaseTrader::stop()
{
}

// protocols::Trader
void CoinbaseTrader::listen(const std::string_view& buffer)
{
     if (d_strategy) {
         d_strategy->handleNewData(buffer);
     }
}

} // traders
} // crypto_trader
