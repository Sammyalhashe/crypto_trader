#include "hodl.h"

#include <boost/asio/buffers_iterator.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>

namespace crypto_trader {
namespace strategies {

namespace {
using json = nlohmann::json;            // from <nlohmann/json.hpp>
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
: d_trades()
, d_basisMarketPrice()
, d_config(config)
{
}

HodlStrategy::~HodlStrategy()
{
}

// MANIPULATORS
void HodlStrategy::handleNewData(const std::string_view &buffer)
{
    using namespace boost::asio;
    try {
        auto data = nlohmann::json::parse(buffer);
        std::cout << "GOT: " << data << '\n';
        auto type = data["type"];

        if (type == "ticker") {
            float price = std::stof(std::string(data["price"]));
            std::cout << "price: " << price << '\n';
            goOverTradesAtPrice(price, std::string(data["time"]));
        }

    }
    catch (json::parse_error& e) {
        std::cerr << e.what() << '\n';
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
                BuyConfig config {.d_timestamp = timestamp,
                                  .d_price     = price,
                                  .d_buyAgain  = true};
                buy(config);
            }
            d_basisMarketPrice.reset();
            return;
        }

        switch (d_config.initStrategy()) {
            case HodlStrategyConfig::e_BUY_IMMEDIATELY: {
                BuyConfig config {.d_timestamp = timestamp,
                                  .d_price     = price,
                                  .d_buyAgain  = true};
                buy(config);
            } break;
            case HodlStrategyConfig::e_SET_BASIS_PRICE: {
                assert(!d_basisMarketPrice);
                d_basisMarketPrice = price;
            } break;
            default: {
                // FIXME: What do here?
            } break;
        }
        return;
    }

    for (auto& trade: d_trades) {
        if (price >= computeXPercentUp(trade.d_price,
                                       d_config.percentUp()))
        { 
            SellConfig config{.d_trade = trade};
            sell(config);
        }
        else if (price <= computeXPercentDown(trade.d_price,
                                              d_config.percentDown())
                 && !trade.d_boughtAgain)
        {
            trade.d_boughtAgain = true;
            BuyConfig config {.d_timestamp = timestamp,
                              .d_price     = price,
                              .d_buyAgain  = false};
            buy(config);
        }
    }
}

bool HodlStrategy::buy(const BuyConfig& config)
{
    // TODO: Implement this somehow
    std::cout << "BUY at price " << config.d_price << '\n';
    d_trades.emplace_back();
    auto& back = d_trades.back();
    back.d_timestamp = config.d_timestamp;
    back.d_boughtAgain = !config.d_buyAgain;
    back.d_price = config.d_price;
    return true;
}

bool HodlStrategy::sell(const SellConfig& config)
{
    // TODO: Implement this somehow
    std::cout << "SELL at price " << config.d_trade.d_price << '\n';
    return true;
}

} // strategies 
} // crypto_trader
