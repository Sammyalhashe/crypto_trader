#include "paper_trading_executor.h"

#include "../common/jsonutils.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <ctime>

namespace crypto_trader {
namespace executors {

PaperTradingExecutor::PaperTradingExecutor(const PaperTradingExecutorConfig& config)
: d_balance(config.initialBalance())
, d_holdings(0.0f)
, d_config(config)
, d_basisMarketPrice(std::nullopt)
{
    SPDLOG_INFO("PaperTradingExecutor for product {} created with initial balance {}",
                 d_config.product(),
                 d_config.initialBalance());
}

bool PaperTradingExecutor::buy(const std::string_view& product, double quantity) {
    if (product != d_config.product()) {
        spdlog::warn("Attempted to buy product {} but executor is for {}", product, d_config.product());
        return false;
    }

    if (!d_basisMarketPrice.has_value()) {
        spdlog::warn("Cannot execute buy, basis market price not set.");
        return false;
    }

    float price = d_basisMarketPrice.value();
    float cost = price * quantity;
    float commission = cost * d_config.commissionRate();
    float totalCost = cost + commission;

    if (d_balance >= totalCost) {
        d_balance -= totalCost;
        d_holdings += quantity;
        d_trades.push_back({"", price, (float)quantity}); // Timestamp will be set by handleNewData
        SPDLOG_INFO("PaperTrade BUY: Product={}, Quantity={}, Price={}, TotalCost={}, Balance={}, Holdings={}",
                     product,
                     quantity,
                     price,
                     totalCost,
                     d_balance,
                     d_holdings);
        return true;
    } else {
        spdlog::warn("PaperTrade BUY: Insufficient balance. Product={}, Quantity={}, Price={}, TotalCost={}, Balance={}",
                     product,
                     quantity,
                     price,
                     totalCost,
                     d_balance);
        return false;
    }
}

bool PaperTradingExecutor::sell(const std::string_view& product, double quantity) {
    if (product != d_config.product()) {
        spdlog::warn("Attempted to sell product {} but executor is for {}", product, d_config.product());
        return false;
    }

    if (!d_basisMarketPrice.has_value()) {
        spdlog::warn("Cannot execute sell, basis market price not set.");
        return false;
    }

    float price = d_basisMarketPrice.value();
    float revenue = price * quantity;
    float commission = revenue * d_config.commissionRate();
    float netRevenue = revenue - commission;

    if (d_holdings >= quantity) {
        d_balance += netRevenue;
        d_holdings -= quantity;
        d_trades.push_back({"", price, -(float)quantity}); // Timestamp will be set by handleNewData
        SPDLOG_INFO("PaperTrade SELL: Product={}, Quantity={}, Price={}, NetRevenue={}, Balance={}, Holdings={}",
                     product,
                     quantity,
                     price,
                     netRevenue,
                     d_balance,
                     d_holdings);
        return true;
    } else {
        spdlog::warn("PaperTrade SELL: Insufficient holdings. Product={}, Quantity={}, Price={}, Holdings={}",
                     product,
                     quantity,
                     price,
                     d_holdings);
        return false;
    }
}

double PaperTradingExecutor::getBalance(const std::string_view& currency) const {
    if (currency == "USD") { // Assuming USD is the base currency for balance
        return d_balance;
    }
    spdlog::warn("getBalance for unsupported currency: {}", currency);
    return 0.0;
}

double PaperTradingExecutor::getPosition(const std::string_view& product) const {
    if (product == d_config.product()) {
        return d_holdings;
    }
    spdlog::warn("getPosition for unsupported product: {}", product);
    return 0.0;
}

void PaperTradingExecutor::handleNewData(const nlohmann::json& data) {
    if (data.contains("product_id") && data.at("product_id") == d_config.product() &&
        data.contains("type") && data.at("type") == "ticker") {
        float price = common::value_or(data, "price", 0.0f).get<float>();
        std::string timestamp = common::value_or(data, "time", "").get<std::string>();
        processTickerData(price, timestamp);
    }
}

void PaperTradingExecutor::processTickerData(float price, const std::string_view& timestamp) {
    if (!d_basisMarketPrice.has_value()) {
        // The only initial strategy is to set the basis price, no immediate buying.
        d_basisMarketPrice = price;
        SPDLOG_INFO("Initial set basis price strategy: basis price set to {} for {}", price, d_config.product());
    } else {
        d_basisMarketPrice = price; // Update current market price
    }

    // Update timestamps for any pending trades (this logic assumes trades are only pending
    // for one market data cycle, which might be overly simplistic but works for now).
    // A more robust solution would associate a timestamp with each trade at the time of execution.
    for (auto& trade : d_trades) {
        if (trade.d_timestamp.empty()) {
            trade.d_timestamp = timestamp;
        }
    }
}

} // namespace executors
} // namespace crypto_trader
