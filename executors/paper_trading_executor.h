#ifndef INCLUDED_PAPER_TRADER_EXECUTOR
#define INCLUDED_PAPER_TRADER_EXECUTOR

#include "../common/Event.h"
#include "../common/types.h"
#include "../protocols/executor.h"
#include "../traders/event_position_manager.h"

#include <spdlog/spdlog.h>

#include <optional>
#include <string_view>
#include <unordered_map>

namespace crypto_trader {
namespace executors {

class PaperTradingExecutorConfig {
  public:
    // PUBLIC TYPES
  private:
    // PRIVATE DATA
    // Initial balance for paper trading
    double d_initialBalance;
    // Commission percentage per trade
    double d_commissionRate;

  public:
    // MANIPULATORS

    PaperTradingExecutorConfig& setInitialBalance(double initialBalance);
    PaperTradingExecutorConfig& setCommissionRate(double commissionRate);

    // ACCESSORS

    double initialBalance() const;
    double commissionRate() const;
}; // PaperTradingExecutorConfig

struct PaperTrade {
    // PUBLIC DATA
    // The product that was traded
    std::string d_symbol;
    // Side of the trade
    common::Side d_side;
    // time when the trade was finalized
    std::string d_timestamp;
    // price at which the trade was executed
    double d_price;
    // amount of product bought/sold
    double d_amount;
    // commission paid on trade
    double d_commission;
}; // PaperTrade

template <common::MarketData T>
class PaperTradingExecutor : public protocols::Executor<T> {

  private:
    // PRIVATE DATA
    // Current cash balance
    double d_balance;
    // Manages positions for different products
    traders::EventPositionManager& d_positionManager;
    // the last market prices recorded for each product
    std::unordered_map<std::string, T> d_lastMarketPrices;
    // Config for the paper trader strategy.
    PaperTradingExecutorConfig d_config;

  public:
    // CREATORS
    PaperTradingExecutor(const PaperTradingExecutorConfig& config,
                         traders::EventPositionManager&    positionManager);
    ~PaperTradingExecutor() = default;

    // MANIPULATORS
    common::TradeResult buy(const std::string_view& product,
                            double                  quantity) override;
    common::TradeResult sell(const std::string_view& product,
                             double                  quantity) override;
    double getBalance(const std::string_view& currency) const override;
    std::optional<double>
         getPosition(const std::string_view& product) const override;
    void processTickerData(const std::string_view& product,
                           double                  price,
                           const T::Timestamp&     timestamp) override;

    // Get the total realized Profit and Loss for the configured product.
    double getRealizedPnl(const std::string_view& product) const;

    // ACCESSORS
    // Return the current balance
    double balance() const;

    // Return either the realized or unrealized pnl for the given `product`
    // depending on the passed in 'realize' boolean (default = false).
    std::optional<double> pnl(const std::string_view& product,
                              bool                    realize) const;

    // Get the average cost basis for the given `product`
    std::optional<double>
    getAverageCostBasis(const std::string_view& product) const;

}; // PaperTradingExecutor

// INLINE DEFINITIONS
// class PaperTradingExecutorConfig

inline PaperTradingExecutorConfig&
PaperTradingExecutorConfig::setInitialBalance(double initialBalance)
{
    d_initialBalance = initialBalance;
    return *this;
}

inline PaperTradingExecutorConfig&
PaperTradingExecutorConfig::setCommissionRate(double commissionRate)
{
    d_commissionRate = commissionRate;
    return *this;
}

inline double PaperTradingExecutorConfig::initialBalance() const
{
    return d_initialBalance;
}

inline double PaperTradingExecutorConfig::commissionRate() const
{
    return d_commissionRate;
}

// class PaperTradingExecutor

// ACCESSORS
template <common::MarketData T>
double PaperTradingExecutor<T>::balance() const
{
    return d_balance;
}

template <common::MarketData T>
PaperTradingExecutor<T>::PaperTradingExecutor(
    const PaperTradingExecutorConfig& config,
    traders::EventPositionManager&    positionManager)
: d_balance(config.initialBalance())
, d_positionManager(positionManager)
, d_lastMarketPrices()
, d_config(config)
{
}

template <common::MarketData T>
common::TradeResult
PaperTradingExecutor<T>::buy(const std::string_view& product, double quantity)
{
    auto it = d_lastMarketPrices.find(std::string(product));
    if (it == d_lastMarketPrices.end()) {
        std::stringstream ss;
        ss << std::format("Cannot execute buy for product {}, basis "
                          "market price not set.",
                          product);
        SPDLOG_WARN(ss.str());
        return {false, 0.0, 0.0, ss.str()};
    }

    const auto& marketData = it->second;
    double      price      = marketData.d_price;
    double      cost       = price * quantity;
    double      commission = cost * d_config.commissionRate();
    double      totalCost  = cost + commission;

    if (d_balance >= totalCost) {
        d_balance -= totalCost;
        Event e = {std::string(product),
                   quantity,
                   price,
                   EventType::ORDER_FILLED,
                   {},
                   marketData.d_timestamp};
        d_positionManager.submit_event(e);
        SPDLOG_INFO("PaperTrade BUY: Product={}, Quantity={}, Price={}, "
                    "TotalCost={}, Balance={}, Holdings={}",
                    product,
                    quantity,
                    price,
                    totalCost,
                    d_balance,
                    d_positionManager.currentHoldings(product).value());
        return {true, price, commission, ""};
    }
    else {
        std::stringstream ss;
        ss << std::format("PaperTrade BUY: Insufficient balance. Product={}, "
                          "Quantity={}, Price={}, TotalCost={}, Balance={}",
                          product,
                          quantity,
                          price,
                          totalCost,
                          d_balance);
        SPDLOG_WARN(ss.str());
        return {false, 0.0, 0.0, ss.str()};
    }
}

template <common::MarketData T>
common::TradeResult
PaperTradingExecutor<T>::sell(const std::string_view& product, double quantity)
{
    auto it = d_lastMarketPrices.find(std::string(product));
    if (it == d_lastMarketPrices.end()) {
        std::stringstream ss;
        ss << std::format("Cannot execute sell for product {}, basis "
                          "market price not set.",
                          product);
        SPDLOG_WARN(ss.str());
        return {false, 0.0, 0.0, ss.str()};
    }

    const auto& marketData = it->second;
    double      price      = marketData.d_price;
    double      revenue    = price * quantity;
    double      commission = revenue * d_config.commissionRate();
    double      netRevenue = revenue - commission;

    auto holdings = d_positionManager.currentHoldings(product);
    if (holdings.has_value() && holdings.value() >= quantity) {
        d_balance += netRevenue;
        Event e = {std::string(product),
                   -quantity,
                   price,
                   EventType::ORDER_FILLED,
                   {},
                   marketData.d_timestamp};
        d_positionManager.submit_event(e);
        std::stringstream ss;
        ss << std::format(
            "PaperTrade SELL: Product={}, Quantity={}, Price={}, "
            "NetRevenue={}, "
            "Balance={}, Holdings={}, Realized PnL={}",
            product,
            quantity,
            price,
            netRevenue,
            d_balance,
            d_positionManager.currentHoldings(product).value(),
            0.0);
        SPDLOG_INFO(ss.str());
        return {true, price, commission, ""};
    }
    else {
        std::stringstream ss;
        ss << std::format(
            "PaperTrade SELL: Insufficient holdings. "
            "Product={}, Quantity={}, Price={}, Holdings={}",
            product,
            quantity,
            price,
            d_positionManager.currentHoldings(product).value_or(0.0));
        SPDLOG_WARN(ss.str());
        return {false, 0.0, 0.0, ss.str()};
    }
}

template <common::MarketData T>
void PaperTradingExecutor<T>::processTickerData(
    const std::string_view& product,
    double                  price,
    const T::Timestamp&     timestamp)
{
    d_lastMarketPrices[std::string(product)] = {
        std::string(product), price, timestamp};
    SPDLOG_INFO("Updated basis price for {} to {} at timestamp={}",
                product,
                price,
                timestamp);
}

template <common::MarketData T>
double
PaperTradingExecutor<T>::getBalance(const std::string_view& currency) const
{
    if (currency == "USD") { // Assuming USD is the base currency for balance
        return d_balance;
    }
    SPDLOG_WARN("getBalance for unsupported currency: {}", currency);
    return 0.0;
}

template <common::MarketData T>
std::optional<double>
PaperTradingExecutor<T>::getPosition(const std::string_view& product) const
{
    return d_positionManager.currentHoldings(std::string(product));
}

template <common::MarketData T>
std::optional<double>
PaperTradingExecutor<T>::pnl(const std::string_view& product,
                             bool                    realize) const
{
    if (realize) {
        return d_positionManager.realizedPnl(std::string(product));
    }

    auto it = d_lastMarketPrices.find(std::string(product));
    if (it == d_lastMarketPrices.end()) {
        SPDLOG_ERROR("Cannot get unrealized pnl for product {}, basis market "
                     "price not set.",
                     product);
        return std::nullopt;
    }

    return d_positionManager.unrealizedPnl(std::string(product),
                                           it->second.d_price);
}

template <common::MarketData T>
std::optional<double> PaperTradingExecutor<T>::getAverageCostBasis(
    const std::string_view& product) const
{
    return d_positionManager.averageCostBasis(std::string(product));
}

} // namespace executors
} // namespace crypto_trader

#endif // INCLUDED_PAPER_TRADER_EXECUTOR
