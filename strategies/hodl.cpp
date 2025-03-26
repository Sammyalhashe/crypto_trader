#include "hodl.h"

#include <boost/asio/buffers_iterator.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <iostream>
#include <string>

namespace crypto_trader {
namespace strategies {

namespace {
using json = nlohmann::json; // from <nlohmann/json.hpp>
                             // NOTE: `json` is a type not
                             // namespace

// FREE FUNCTIONS
double computeXPercentUp(double price, float percent)
{
    return price * (1 + percent / 100);
}

double computeXPercentDown(double price, float percent)
{
    return price * (1 - percent / 100);
}

} // unnamed namespace

// class hodlStrategy

// CREATORS
HodlStrategy::HodlStrategy(const HodlStrategyConfig& config)
: protocols::Strategy(config.emit())
, d_tradeIdBasis(0)
, d_trades()
, d_basisMarketPrice()
, d_symbolToLastPrice()
, d_config(config)
{
}

HodlStrategy::~HodlStrategy() {}

// MANIPULATORS
// protocols::strategy
void HodlStrategy::handleNewData(const nlohmann::json& data)
{
    auto type = data["type"];

    if (type == "ticker") {
        double price = std::stof(std::string(data["price"]));
        d_symbolToLastPrice[data["symbol"]] = price;
        goOverTradesAtPrice(data["symbol"], price, std::string(data["time"]));
    }
}

// ACCESSORS
// protocols::strategy
double HodlStrategy::totalPosition() const
{
    double position = d_config.fundsAvailable();

    return position;
}

// PRIVATE MANIPULATORS
void HodlStrategy::goOverTradesAtPrice(const std::string& symbol,
                                       double             price,
                                       const std::string& timestamp)
{
    if (d_trades.empty()) {
        if (d_basisMarketPrice) {
            if (price <= computeXPercentDown(d_basisMarketPrice.value(),
                                             d_config.percentDown()))
            {
                BuyConfig config{.d_symbol    = symbol,
                                 .d_timestamp = timestamp,
                                 .d_price     = price,
                                 .d_buyAgain  = true};
                buy(config);
            }
            d_basisMarketPrice.reset();
            return;
        }

        switch (d_config.initStrategy()) {
        case HodlStrategyConfig::e_BUY_IMMEDIATELY: {
            BuyConfig config{.d_symbol    = symbol,
                             .d_timestamp = timestamp,
                             .d_price     = price,
                             .d_buyAgain  = true};
            buy(config);
        } break;
        case HodlStrategyConfig::e_SET_BASIS_PRICE: {
            assert(!d_basisMarketPrice);
            d_basisMarketPrice = price;
        } break;
        default: {
            std::stringstream ss;
            ss << d_config.initStrategy();
            spdlog::error("Recieved unknown initStrategy: {}", ss.str());
            assert(false);
        } break;
        }
        return;
    }

    using SymbolIT    = TradesForSymbol::iterator;
    using TradesIT    = TradeMap::iterator;
    SymbolIT symbolIt = d_trades.begin();
    for (; d_trades.end() != symbolIt; ++symbolIt) {
        TradesIT it = symbolIt->second.begin();
        for (; symbolIt->second.end() != it;) {
            if (price >=
                computeXPercentUp(it->second.d_price, d_config.percentUp()))
            {
                SellConfig config{
                    .d_symbol = symbol, .d_trade = it++, .d_price = price};
                sell(config);
                continue;
            }
            else if (price <= computeXPercentDown(it->second.d_price,
                                                  d_config.percentDown()) &&
                     !it->second.d_boughtAgain)
            {
                it->second.d_boughtAgain = true;
                BuyConfig config{.d_symbol    = symbol,
                                 .d_timestamp = timestamp,
                                 .d_price     = price,
                                 .d_buyAgain  = false};
                buy(config);
            }
            ++it;
        }
    }
}

void HodlStrategy::buy(const BuyConfig& config)
{
    if (d_config.buyAmount() > d_config.fundsAvailable()) {
        spdlog::warn("BUY not executed due to lack of funds");
        return;
    }
    spdlog::info("BUY at price {}", config.d_price);
    auto& back         = d_trades[++d_tradeIdBasis];
    back.d_timestamp   = config.d_timestamp;
    back.d_boughtAgain = !config.d_buyAgain;
    back.d_price       = config.d_price;

    d_config.setFundsAvailable(d_config.fundsAvailable() -
                               d_config.buyAmount());

    if (d_emit) {
        common::Action action{.d_type = common::Action::e_BUY};
        d_emit(action);
    }
}

void HodlStrategy::sell(const SellConfig& config)
{
    spdlog::info("SELL {} at price {} for trade bought at {}",
                 config.d_symbol,
                 config.d_price,
                 config.d_trade->second.d_price);
    d_trades.erase(config.d_trade);

    if (d_config.initStrategy() == HodlStrategyConfig::e_SET_BASIS_PRICE &&
        d_trades.empty())
    {
        assert(!d_basisMarketPrice);
        d_basisMarketPrice = config.d_price;
    }

    if (d_emit) {
        common::Action action{.d_type = common::Action::e_SELL};
        d_emit(action);
    }
}

} // namespace strategies
} // namespace crypto_trader
