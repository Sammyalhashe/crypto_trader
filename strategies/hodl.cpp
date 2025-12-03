#include "hodl.h"

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

double convertFromUSDToProductAmount(double price, double amount)
{
    // ie) ETH-USD = 2800, amount = 100
    // ETH = 100 / 2800 = 1 / 28 = 0.035 ETH

    return amount / price;
}

} // unnamed namespace

// class hodlStrategy

// CREATORS
HodlStrategy::HodlStrategy(const HodlStrategyConfig& config)
: protocols::Strategy(config.emit())
, d_tradeIdBasis(0)
, d_tradesForProduct()
, d_basisMarketPrices()
, d_config(config)
{
}

HodlStrategy::~HodlStrategy() {}

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

// PRIVATE MANIPULATORS
void HodlStrategy::goOverTradesAtPrice(const std::string_view& product,
                                       double                  price,
                                       const std::string_view& timestamp)
{
    auto& trades = d_tradesForProduct[std::string(product)];
    if (trades.empty()) {
        auto it = d_basisMarketPrices.find(std::string(product));
        if (it != d_basisMarketPrices.end()) {
            if (price <=
                computeXPercentDown(it->second, d_config.percentDown()))
            {
                SPDLOG_INFO("HODL: Buy due to price drop below basis");
                BuyConfig config{.d_product   = std::string(product),
                                 .d_timestamp = timestamp,
                                 .d_price     = price,
                                 .d_buyAgain  = true};
                buy(config);
            }
            d_basisMarketPrices.erase(it);
            return;
        }

        switch (d_config.initStrategy()) {
        case HodlStrategyConfig::e_BUY_IMMEDIATELY: {
            SPDLOG_INFO("HODL: Buy due to buy_immediately");
            BuyConfig config{.d_product   = std::string(product),
                             .d_timestamp = timestamp,
                             .d_price     = price,
                             .d_buyAgain  = true};
            buy(config);
        } break;
        case HodlStrategyConfig::e_SET_BASIS_PRICE: {
            assert(d_basisMarketPrices.find(std::string(product)) ==
                   d_basisMarketPrices.end());
            d_basisMarketPrices[std::string(product)] = price;
        } break;
        default: {
            std::stringstream ss;
            ss << d_config.initStrategy();
            SPDLOG_ERROR("Recieved unknown initStrategy: {}", ss.str());
            assert(false);
        } break;
        }
        return;
    }

    using IT = TradeMap::iterator;
    IT it    = trades.begin();
    for (; trades.end() != it;) {
        if (price >=
            computeXPercentUp(it->second.d_price, d_config.percentUp()))
        {
            SPDLOG_INFO("HODL: Sell as price rose");
            SellConfig config{.d_product = it->second.d_product,
                              .d_trade   = it++,
                              .d_price   = price};
            sell(config);
            continue;
        }
        else if (price <= computeXPercentDown(it->second.d_price,
                                              d_config.percentDown()) &&
                 !it->second.d_boughtAgain)
        {
            SPDLOG_INFO("HODL: Buy as price dropped and we buy again");
            it->second.d_boughtAgain = true;
            BuyConfig config{.d_product   = std::string(product),
                             .d_timestamp = timestamp,
                             .d_price     = price,
                             .d_buyAgain  = false};
            buy(config);
        }
        ++it;
    }
}

void HodlStrategy::buy(const BuyConfig& config)
{
    SPDLOG_INFO("HODL: BUY {} at price {}", config.d_product, config.d_price);
    auto& back       = d_tradesForProduct[config.d_product][++d_tradeIdBasis];
    back.d_timestamp = config.d_timestamp;
    back.d_boughtAgain = !config.d_buyAgain;
    back.d_price       = config.d_price;
    back.d_product     = config.d_product;

    if (d_emit) {
        common::Action action{.d_type     = common::Side::e_BUY,
                              .d_product  = config.d_product,
                              .d_quantity = convertFromUSDToProductAmount(
                                  back.d_price, d_config.buyAmount())};
        d_emit(action);
    }
}

void HodlStrategy::sell(const SellConfig& config)
{
    SPDLOG_INFO("HODL: SELL {} at price {} for trade bought at {}",
                config.d_product,
                config.d_price,
                config.d_trade->second.d_price);
    d_tradesForProduct[config.d_product].erase(config.d_trade);

    if (d_config.initStrategy() == HodlStrategyConfig::e_SET_BASIS_PRICE &&
        d_tradesForProduct[config.d_product].empty())
    {
        assert(d_basisMarketPrices.find(config.d_product) ==
               d_basisMarketPrices.end());
        d_basisMarketPrices[config.d_product] = config.d_price;
    }

    if (d_emit) {
        common::Action action{
            .d_type     = common::Side::e_SELL,
            .d_product  = config.d_product,
            .d_quantity = convertFromUSDToProductAmount(
                config.d_trade->second.d_price, d_config.buyAmount())};
        d_emit(action);
    }
}

} // namespace strategies
} // namespace crypto_trader
