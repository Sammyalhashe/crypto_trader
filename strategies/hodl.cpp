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
float computeXPercentUp(float price, float percent)
{
    return price * (1 + percent / 100);
}

float computeXPercentDown(float price, float percent)
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
, d_config(config)
{
}

HodlStrategy::~HodlStrategy() {}

// MANIPULATORS
void HodlStrategy::handleNewData(const nlohmann::json& data)
{
    auto type = data["type"];

    if (type == "ticker") {
        float price = std::stof(std::string(data["price"]));
        goOverTradesAtPrice(price, std::string(data["time"]));
    }
}

// PRIVATE MANIPULATORS
void HodlStrategy::goOverTradesAtPrice(float                   price,
                                       const std::string_view& timestamp)
{
    if (d_trades.empty()) {
        if (d_basisMarketPrice) {
            if (price <= computeXPercentDown(d_basisMarketPrice.value(),
                                             d_config.percentDown()))
            {
                BuyConfig config{.d_timestamp = timestamp,
                                 .d_price     = price,
                                 .d_buyAgain  = true};
                buy(config);
            }
            d_basisMarketPrice.reset();
            return;
        }

        switch (d_config.initStrategy()) {
        case HodlStrategyConfig::e_BUY_IMMEDIATELY: {
            BuyConfig config{.d_timestamp = timestamp,
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

    using IT = TradeMap::iterator;
    IT it    = d_trades.begin();
    for (; d_trades.end() != it;) {
        if (price >=
            computeXPercentUp(it->second.d_price, d_config.percentUp()))
        {
            SellConfig config{.d_trade = it++, .d_price = price};
            sell(config);
            continue;
        }
        else if (price <= computeXPercentDown(it->second.d_price,
                                              d_config.percentDown()) &&
                 !it->second.d_boughtAgain)
        {
            it->second.d_boughtAgain = true;
            BuyConfig config{.d_timestamp = timestamp,
                             .d_price     = price,
                             .d_buyAgain  = false};
            buy(config);
        }
        ++it;
    }
}

void HodlStrategy::buy(const BuyConfig& config)
{
    spdlog::info("BUY at price {}", config.d_price);
    auto& back         = d_trades[++d_tradeIdBasis];
    back.d_timestamp   = config.d_timestamp;
    back.d_boughtAgain = !config.d_buyAgain;
    back.d_price       = config.d_price;

    if (d_emit) {
        common::Action action{.d_type = common::Action::e_BUY};
        d_emit(action);
    }
}

void HodlStrategy::sell(const SellConfig& config)
{
    spdlog::info("SELL at price {} for trade bought at {}",
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
