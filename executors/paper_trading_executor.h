#ifndef INCLUDED_PAPER_TRADER_EXECUTOR
#define INCLUDED_PAPER_TRADER_EXECUTOR

#include "../common/Event.h"
#include "../common/math.h"
#include "../common/types.h"
#include "../protocols/executor.h"
#include "../traders/event_position_manager.h"

#include <spdlog/spdlog.h>

#include <optional>
#include <string_view>
#include <unordered_map>

namespace crypto_trader {
namespace executors {

/**
 * @brief Configuration parameters for the PaperTradingExecutor.
 */
class PaperTradingExecutorConfig {
  public:
    // PUBLIC TYPES
  private:
    double d_initialBalance; //!< Initial balance for paper trading.
    double d_commissionRate; //!< Commission percentage per trade.

  public:
    /**
     * @brief Sets the initial balance for paper trading.
     * @param initialBalance The initial balance.
     * @return A reference to the updated configuration object.
     */
    PaperTradingExecutorConfig& setInitialBalance(double initialBalance);

    /**
     * @brief Sets the commission rate for paper trading.
     * @param commissionRate The commission rate (e.g., 0.001 for 0.1%).
     * @return A reference to the updated configuration object.
     */
    PaperTradingExecutorConfig& setCommissionRate(double commissionRate);

    /**
     * @brief Gets the initial balance.
     * @return The initial balance.
     */
    double initialBalance() const;

    /**
     * @brief Gets the commission rate.
     * @return The commission rate.
     */
    double commissionRate() const;
}; // PaperTradingExecutorConfig

/**
 * @brief Represents a simulated trade executed by the PaperTradingExecutor.
 */
struct PaperTrade {
    std::string d_symbol;    //!< The product that was traded.
    common::Side d_side;     //!< Side of the trade (Buy or Sell).
    std::string d_timestamp; //!< Time when the trade was finalized.
    double d_price;          //!< Price at which the trade was executed.
    double d_amount;         //!< Amount of product bought/sold.
    double d_commission;     //!< Commission paid on the trade.
}; // PaperTrade

/**
 * @brief A concrete implementation of the Executor protocol for paper (simulated) trading.
 *
 * This executor manages an internal cash balance and positions, simulating trade executions
 * without interacting with real exchanges.
 * @tparam T The MarketData type that the executor processes.
 */
template <common::MarketData T>
class PaperTradingExecutor : public protocols::Executor<T> {

  private:
    double d_balance;                                    //!< Current cash balance in the base currency (e.g., USD).
    traders::EventPositionManager& d_positionManager; //!< Manages positions for different products.
    std::unordered_map<std::string, T> d_lastMarketPrices; //!< The last market prices recorded for each product.
    PaperTradingExecutorConfig d_config;                 //!< Configuration for the paper trader.

  public:
    /**
     * @brief Constructs a PaperTradingExecutor.
     * @param config The configuration for the executor.
     * @param positionManager A reference to the EventPositionManager for tracking positions.
     */
    PaperTradingExecutor(const PaperTradingExecutorConfig& config,
                         traders::EventPositionManager&    positionManager);

    /**
     * @brief Destructor for PaperTradingExecutor.
     */
    ~PaperTradingExecutor() = default;

    /**
     * @brief Executes a simulated buy order.
     * @param product The symbol of the product to buy.
     * @param quantity The amount of the product to buy.
     * @return The result of the simulated trade.
     */
    common::TradeResult buy(const std::string_view& product,
                            double                  quantity) override;

    /**
     * @brief Executes a simulated sell order.
     * @param product The symbol of the product to sell.
     * @param quantity The amount of the product to sell.
     * @return The result of the simulated trade.
     */
    common::TradeResult sell(const std::string_view& product,
                             double                  quantity) override;

    /**
     * @brief Gets the current balance of a currency.
     * @param currency The symbol of the currency (e.g., "USD").
     * @return The current balance.
     */
    double getBalance(const std::string_view& currency) const override;

    /**
     * @brief Gets the current position for a given product.
     * @param product The symbol of the product.
     * @return An optional containing the quantity held, or empty if no position.
     */
    std::optional<double>
    getPosition(const std::string_view& product) const override;

    /**
     * @brief Processes new ticker data to update internal market prices.
     * @param product The symbol of the product for the ticker.
     * @param price The latest price of the product.
     * @param timestamp The timestamp of the ticker data.
     */
    void processTickerData(const std::string_view& product,
                           double                  price,
                           const T::Timestamp&     timestamp) override;

    /**
     * @brief Gets the total realized Profit and Loss for the configured product.
     * @param product The symbol of the product.
     * @return The total realized PnL.
     */
    double getRealizedPnl(const std::string_view& product) const;

    /**
     * @brief Returns the current cash balance.
     * @return The current balance.
     */
    double balance() const;

    /**
     * @brief Returns either the realized or unrealized PnL for the given product.
     * @param product The symbol of the product.
     * @param realize If true, returns realized PnL; otherwise, returns unrealized PnL.
     * @return An optional containing the PnL, or empty if not applicable.
     */
    std::optional<double> pnl(const std::string_view& product,
                              bool                    realize) const;

    /**
     * @brief Gets the average cost basis for a given product.
     * @param product The symbol of the product.
     * @return An optional containing the average cost basis, or empty if no position.
     */
    std::optional<double>
    getAverageCostBasis(const std::string_view& product) const;

}; // PaperTradingExecutor

// INLINE DEFINITIONS
// class PaperTradingExecutorConfig

/**
 * @brief Sets the initial balance for paper trading.
 * @param initialBalance The initial balance.
 * @return A reference to the updated configuration object.
 */
inline PaperTradingExecutorConfig&
PaperTradingExecutorConfig::setInitialBalance(double initialBalance)
{
    d_initialBalance = initialBalance;
    return *this;
}

/**
 * @brief Sets the commission rate for paper trading.
 * @param commissionRate The commission rate (e.g., 0.001 for 0.1%).
 * @return A reference to the updated configuration object.
 */
inline PaperTradingExecutorConfig&
PaperTradingExecutorConfig::setCommissionRate(double commissionRate)
{
    d_commissionRate = commissionRate;
    return *this;
}

/**
 * @brief Gets the initial balance.
 * @return The initial balance.
 */
inline double PaperTradingExecutorConfig::initialBalance() const
{
    return d_initialBalance;
}

/**
 * @brief Gets the commission rate.
 * @return The commission rate.
 */
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

    if (crypto_trader::common::Math::isGreaterOrEqual(d_balance, totalCost)) {
        d_balance -= totalCost;
        common::Event e = {std::string(product),
                           quantity,
                           price,
                           common::EventType::ORDER_FILLED,
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
    if (holdings.has_value() && crypto_trader::common::Math::isGreaterOrEqual(
                                    holdings.value(), quantity))
    {
        d_balance += netRevenue;
        common::Event e = {std::string(product),
                           -quantity,
                           price,
                           common::EventType::ORDER_FILLED,
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