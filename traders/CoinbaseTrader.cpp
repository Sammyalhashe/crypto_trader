#include "CoinbaseTrader.h"

#include "../adaptors/coinbase_websocket_client.h"
#include "../adaptors/coinbase_websocket_client_async.h"
#include "../common/jsonutils.h"
#include "../strategies/hodl.h"
#include "../strategies/index.h"

#include "../executors/paper_trading_executor.h"
#include "../executors/real_trading_executor.h"

#include <boost/beast/ssl.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <string>

namespace crypto_trader {
namespace traders {

namespace {
using json = nlohmann::json; // from <nlohmann/json.hpp>
                             // NOTE: `json` is a type not
                             // namespace

void buildCoinbaseWebsocketMessage(nlohmann::json             *message,
                                   const std::string&          type,
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
    const std::shared_ptr<std::atomic_bool>& isRunning, bool paperTrading)
: d_channels()
, d_products()
, d_strategy(strategies::e_NONE)
, d_strategyConfig()
, d_url()
, d_numThreads(1)
, d_clientType(CoinbaseTraderConfig::ClientType::SYNC)
, d_isRunning(isRunning)
, d_paperTrading(paperTrading)
{
}

// class CoinbaseTrader

// STATIC DATA
const char *CoinbaseTrader::s_databaseFile =
    "/var/tmp/crypto_trader/coinbase_trader_data";

void CoinbaseTrader::initWebsocketClient()
{
    nlohmann::json result;
    buildCoinbaseWebsocketMessage(&result, "subscribe", d_config);
    switch (d_config.clientType()) {
    case CoinbaseTraderConfig::ClientType::SYNC: {
        adaptors::CoinbaseWebSocketClientConfig coinbaseWebSocketConfig(
            d_config.url(),
            result,
            std::bind(&CoinbaseTrader::listen, this, std::placeholders::_1),
            d_config.isRunning());
        d_webSocketClient =
            std::make_unique<adaptors::CoinbaseWebSocketClient>(
                coinbaseWebSocketConfig);
    } break;
    case CoinbaseTraderConfig::ClientType::ASYNC: {
        adaptors::CoinbaseWebSocketClientAsyncConfig coinbaseWebSocketConfig(
            d_config.url(),
            result,
            std::bind(&CoinbaseTrader::listen, this, std::placeholders::_1),
            5,
            std::chrono::seconds{2},
            d_config.isRunning());

        d_webSocketClient =
            std::make_shared<adaptors::CoinbaseWebSocketClientAsync>(
                coinbaseWebSocketConfig);
    } break;
    case CoinbaseTraderConfig::ClientType::COUNT: {
        /* noop */
    } /* fall through */;
    default: {
        std::stringstream ss;
        ss << d_config.clientType();
        spdlog::error("unknown client type: {}", ss.str());
    } break;
    }
}

// CREATORS
CoinbaseTrader::CoinbaseTrader(const CoinbaseTraderConfig& config)
: d_webSocketClient()
, d_strategy()
, d_threadPool(config.numThreads())
, d_isStopped(true)
, d_database()
, d_mutex()
, d_config(config)
, d_executor()
, d_lastSequenceNumbers()
{
    switch (d_config.strategy()) {
    case strategies::TradingStrategy::e_HODL: {
        initWebsocketClient();

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

    if (d_config.paperTrading()) {
        executors::PaperTradingExecutorConfig paperTradingConfig;
        const auto& strategyConfigJson = d_config.strategyConfig();
        paperTradingConfig
            .setInitialBalance(common::value_or(
                strategyConfigJson, "initialBalance", 1000.0f))
            .setCommissionRate(common::value_or(
                strategyConfigJson, "commissionRate", 0.001f)); // 0.1%

        d_executor = std::make_unique<
            executors::PaperTradingExecutor<common::MarketDataCoinbase>>(
            paperTradingConfig);
    }
    else {
        executors::RealTradingExecutorConfig realTradingConfig;
        d_executor = std::make_unique<
            executors::RealTradingExecutor<common::MarketDataCoinbase>>(
            realTradingConfig);
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
        SPDLOG_INFO("after listen");
    }
}

void CoinbaseTrader::stop()
{
    if (d_isStopped) {
        return;
    }

    d_database.save(s_databaseFile);

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
    SPDLOG_INFO("Processing action: {}", ss.str());

    boost::asio::post(d_threadPool,
                      std::bind(&CoinbaseTrader::handleAction, this, action));
}

void CoinbaseTrader::handleAction(const common::Action& action)
{
    std::stringstream ss;
    ss << action.d_type;
    SPDLOG_INFO("Handling action: {}", ss.str());

    if (d_executor) {
        if (action.d_type == common::Side::e_BUY) {
            d_executor->buy(action.d_product, action.d_quantity);
        }
        else if (action.d_type == common::Side::e_SELL) {
            d_executor->sell(action.d_product, action.d_quantity);
        }
        else {
            SPDLOG_WARN("Unsupported action type for executor: {}", ss.str());
        }
    }
    else {
        SPDLOG_ERROR("No executor available to handle action.");
    }
}

void CoinbaseTrader::handleNewData(const std::string_view& buffer)
{
    std::lock_guard<std::mutex> guard(d_mutex); // LOCK
    if (!*d_config.isRunning()) {
        d_threadPool.stop();
        return;
    }
    if (d_strategy) {
        try {
            auto data = nlohmann::json::parse(buffer);

            auto type = data["type"];
            if (type == "ticker") {
                handleTickerMessage(data);
            }
        }
        catch (json::parse_error& e) {
            spdlog::error("{}", e.what());
        }
        catch (json::type_error& e) {
            spdlog::error("{}", e.what());
        }
    }
}

// protocols::Trader
void CoinbaseTrader::listen(const std::string_view& buffer)
{
    std::lock_guard<std::mutex> guard(d_mutex); // LOCK
    if (!*d_config.isRunning() || d_isStopped) {
        d_threadPool.stop();
        return;
    }

    std::string v(buffer);
    boost::asio::post(d_threadPool,
                      std::bind(&CoinbaseTrader::handleNewData, this, v));
}

bool CoinbaseTrader::checkSequenceNumber(const std::string_view& product,
                                         int64_t                 sequence)
{
    std::string product_s;
    auto        it = d_lastSequenceNumbers.find(product_s);

    if (it == d_lastSequenceNumbers.end()) {
        // First message for this product
        d_lastSequenceNumbers[product_s] = sequence;
        return true;
    }

    int64_t expected = it->second + 1;

    if (sequence < expected) {
        SPDLOG_WARN("Out-of-order message for {}: got {}, expected >= {}",
                    product,
                    sequence,
                    expected);
        return false; // Ignore out-of-order messages
    }

    if (sequence > expected) {
        int64_t dropped = sequence - expected;
        SPDLOG_WARN("Dropped {} message(s) for {} (gap: {} to {})",
                    dropped,
                    product,
                    expected,
                    sequence - 1);
        // Continue processing despite gap
    }

    it->second = sequence;
    return true;
}

void CoinbaseTrader::handleTickerMessage(const nlohmann::json& data)
{
    common::MarketDataCoinbase marketData;

    std::string price     = data["price"];
    marketData.d_symbol   = data["product_id"];
    marketData.d_price    = std::stod(price);
    marketData.d_sequence = data["sequence"];

    // Parse ISO 8601 timestamp to Unix milliseconds
    std::string timeStr = data["time"];

    // Check sequence number first
    if (!checkSequenceNumber(marketData.d_symbol, marketData.d_sequence)) {
        SPDLOG_DEBUG("Ignoring out-of-order message for {}",
                     marketData.d_symbol);
        return; // Skip this message
    }

    std::stringstream ss;
    ss << data;

    SPDLOG_INFO("{}", ss.str());

    d_strategy->handleNewData(data);

    marketData.d_timestamp = common::parseISO8601ToMillis(timeStr);

    if (d_executor) {
        d_executor->processTickerData(
            marketData.d_symbol, marketData.d_price, marketData.d_timestamp);
    }

    d_database.add_data(marketData.d_symbol, marketData);
}

} // namespace traders
} // namespace crypto_trader
