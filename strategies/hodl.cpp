#include "hodl.h"
#include "../common/math.h"

#include <boost/asio/buffers_iterator.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <iostream>
#include <string>
#include <string_view>

namespace crypto_trader {
namespace strategies {

namespace {
using json = nlohmann::json; // from <nlohmann/json.hpp>

// FREE FUNCTIONS
double computeXPercentUp(double price, float percent)
{
    return price * (1 + percent / 100);
}

double computeXPercentDown(double price, float percent)
{
    return price * (1 - percent / 100);
}

double convertFromUSDToProductAmount(double price, double amount)
{
    return amount / price;
}

} // unnamed namespace

// CREATORS
HodlStrategy::HodlStrategy(const HodlStrategyConfig&      config,
                           traders::EventPositionManager& positionManager)
: protocols::Strategy(config.emit())
, d_positionManager(positionManager)
, d_config(config)
{
    d_positionManager.register_observer(this);
}

HodlStrategy::~HodlStrategy() { d_positionManager.unregister_observer(this); }

// MANIPULATORS
void HodlStrategy::handleNewData(const nlohmann::json& data)
{
    auto        type    = data["type"];
    std::string product = data["product_id"];

    if (type == "ticker") {
        double price = std::stof(std::string(data["price"]));
        goOverTradesAtPrice(product, price, std::string(data["time"]));
    }
}

void HodlStrategy::on_trade(const common::Trade& trade)
{
    auto& state          = d_symbolStates[trade.d_symbol];
    state.lastBuyPrice   = trade.d_price;
    state.waitingForSell = true;
}

void HodlStrategy::on_position_update(const std::string& symbol,
                                      double             new_position)
{
    auto& state = d_symbolStates[symbol];
    if (crypto_trader::common::Math::isZero(new_position)) {
        state.waitingForSell = false;
        state.hasBoughtAgain = false;
    }
}

// PRIVATE MANIPULATORS
void HodlStrategy::goOverTradesAtPrice(const std::string_view& product_sv,
                                       double                  price,
                                       const std::string_view& timestamp)
{
    std::string product(product_sv);
    auto&       state = d_symbolStates[product];

    if (!state.waitingForSell) {
        if (d_config.initStrategy() == HodlStrategyConfig::e_BUY_IMMEDIATELY) {
            buy(product, price, std::string(timestamp));
        }
        else {
            // e_SET_BASIS_PRICE
            if (crypto_trader::common::Math::isZero(state.lastBuyPrice)) {
                state.lastBuyPrice = price;
            }
            else if (crypto_trader::common::Math::isLessOrEqual(
                         price,
                         computeXPercentDown(state.lastBuyPrice,
                                             d_config.percentDown())))
            {
                buy(product, price, std::string(timestamp));
            }
        }
    }
    else {
        auto currentPosition =
            d_positionManager.currentHoldings(product).value_or(0.0);
        if (crypto_trader::common::Math::isGreater(currentPosition, 0.0)) {
            if (crypto_trader::common::Math::isGreaterOrEqual(
                    price,
                    computeXPercentUp(state.lastBuyPrice,
                                      d_config.percentUp())))
            {
                sell(product, price, std::string(timestamp));
            }
            else if (crypto_trader::common::Math::isLessOrEqual(
                         price,
                         computeXPercentDown(state.lastBuyPrice,
                                             d_config.percentDown())) &&
                     !state.hasBoughtAgain)
            {
                buy(product, price, std::string(timestamp));
                state.hasBoughtAgain = true;
            }
        }
    }
}

void HodlStrategy::buy(const std::string& product,
                       double             price,
                       const std::string& timestamp)
{
    SPDLOG_INFO("HODL: BUY {} at price {}", product, price);
    if (d_config.emit()) {
        common::Action action{.d_type     = common::Side::e_BUY,
                              .d_product  = product,
                              .d_quantity = convertFromUSDToProductAmount(
                                  price, d_config.buyAmount())};
        d_config.emit()(action);
    }
}

void HodlStrategy::sell(const std::string& product,
                        double             price,
                        const std::string& timestamp)
{
    SPDLOG_INFO("HODL: SELL {} at price {}", product, price);
    auto currentPosition =
        d_positionManager.currentHoldings(product).value_or(0.0);
    if (d_config.emit() &&
        crypto_trader::common::Math::isGreater(currentPosition, 0.0))
    {
        common::Action action{.d_type     = common::Side::e_SELL,
                              .d_product  = product,
                              .d_quantity = currentPosition};
        d_config.emit()(action);
    }
}

} // namespace strategies
} // namespace crypto_trader
