#ifndef INCLUDED_PAPER_TRADER_EXECUTOR
#define INCLUDED_PAPER_TRADER_EXECUTOR

#include "../protocols/executor.h"

#include <spdlog/spdlog.h>

#include <iostream>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace crypto_trader {
namespace executors {

class PaperTradingExecutorConfig {
  public:
    // PUBLIC TYPES
    enum InitStrategy {
        e_SET_BASIS_PRICE = 0 // Changed from 1 to 0 as it's the only strategy
    }; // InitStrategy

  private:
    // PRIVATE DATA
    // Action to take when the strategy first starts.
    InitStrategy d_initStrategy;
    // Initial balance for paper trading
    float d_initialBalance;
    // Commission percentage per trade
    float d_commissionRate;
    // The product this paper trader is for
    std::string d_product;

  public:
    // MANIPULATORS
    PaperTradingExecutorConfig& setInitStrategy(const InitStrategy& initStrat);
    PaperTradingExecutorConfig& setInitialBalance(float initialBalance);
    PaperTradingExecutorConfig& setCommissionRate(float commissionRate);
    PaperTradingExecutorConfig& setProduct(const std::string& product);

    // ACCESSORS
    const InitStrategy& initStrategy() const;
    float               initialBalance() const;
    float               commissionRate() const;
    const std::string&  product() const;

}; // PaperTradingExecutorConfig

struct PaperTrade {
    // PUBLIC DATA
    // time when the trade was finalized
    std::string d_timestamp;
    // price at which the trade was executed
    float d_price;
    // amount of product bought/sold
    float d_amount;
}; // PaperTrade

class PaperTradingExecutor : public protocols::Executor {

  private:
    // PRIVATE TYPES
    typedef std::vector<PaperTrade> TradeList;

    // PRIVATE DATA
    // Current cash balance
    float d_balance;
    // Current holdings of the product
    float d_holdings;
    // Config for the paper trader strategy.
    PaperTradingExecutorConfig d_config;
    // List of trades made
    TradeList d_trades;
    // The price that helps us to descern what's the best course of action
    // to take when initially starting or we sold our last position.
    std::optional<float> d_basisMarketPrice;


  public:
    // CREATORS
    PaperTradingExecutor(const PaperTradingExecutorConfig& config);
    ~PaperTradingExecutor() = default;

    // MANIPULATORS
    bool buy(const std::string_view& product, double quantity) override;
    bool sell(const std::string_view& product, double quantity) override;
    double getBalance(const std::string_view& currency) const override;
    double getPosition(const std::string_view& product) const override;
    void handleNewData(const nlohmann::json& data) override;

    // ACCESSORS
    // Return the current balance
    float balance() const;
    // Return the current holdings
    float holdings() const;
    // Return a non-modifiable reference to the list of trades
    const TradeList& trades() const;
    // Return a non-modifiable reference to the basisMarketPrice.
    const std::optional<float>& basisMarketPrice() const;


  private:
    // PRIVATE MANIPULATORS
    void processTickerData(float price, const std::string_view& timestamp);

}; // PaperTradingExecutor

// INLINE DEFINITIONS
// class PaperTradingExecutorConfig

inline PaperTradingExecutorConfig&
PaperTradingExecutorConfig::setInitStrategy(const InitStrategy& initStrat)
{
    d_initStrategy = initStrat;
    return *this;
}

inline PaperTradingExecutorConfig&
PaperTradingExecutorConfig::setInitialBalance(float initialBalance)
{
    d_initialBalance = initialBalance;
    return *this;
}

inline PaperTradingExecutorConfig&
PaperTradingExecutorConfig::setCommissionRate(float commissionRate)
{
    d_commissionRate = commissionRate;
    return *this;
}

inline PaperTradingExecutorConfig&
PaperTradingExecutorConfig::setProduct(const std::string& product)
{
    d_product = product;
    return *this;
}

inline const PaperTradingExecutorConfig::InitStrategy&
PaperTradingExecutorConfig::initStrategy() const
{
    return d_initStrategy;
}

inline float PaperTradingExecutorConfig::initialBalance() const { return d_initialBalance; }

inline float PaperTradingExecutorConfig::commissionRate() const { return d_commissionRate; }

inline const std::string& PaperTradingExecutorConfig::product() const { return d_product; }

// class PaperTradingExecutor

// ACCESSORS
inline float PaperTradingExecutor::balance() const { return d_balance; }

inline float PaperTradingExecutor::holdings() const { return d_holdings; }

inline const PaperTradingExecutor::TradeList& PaperTradingExecutor::trades() const
{
    return d_trades;
}

inline const std::optional<float>& PaperTradingExecutor::basisMarketPrice() const
{
    return d_basisMarketPrice;
}

} // namespace executors
} // namespace crypto_trader

#endif // INCLUDED_PAPER_TRADER_EXECUTOR
