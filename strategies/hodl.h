#ifndef INCLUDED_HODL_STRATEGY
#define INCLUDED_HODL_STRATEGY

#include "../common/types.h"
#include "../protocols/strategy.h"

#include <boost/beast/core.hpp>

#include <iostream>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace crypto_trader {
namespace strategies {

class HodlStrategyConfig {
  public:
    // PUBLIC TYPES
    enum InitStrategy {
        e_BUY_IMMEDIATELY = 0,
        e_SET_BASIS_PRICE = 1
    }; // InitStrategy

  private:
    // PRIVATE DATA
    // Action to take when the strategy first starts.
    InitStrategy d_initStrategy;
    // X percent up to sell for profit
    float d_percentUp;
    // Y percent down to buy if the market dips
    float d_percentDown;
    // Callback called on emitted action by strategy.
    common::Emit d_emit;

  public:
    // MANIPULATORS
    HodlStrategyConfig& setInitStrategy(const InitStrategy& initStrat);
    HodlStrategyConfig& setPercentUp(float percentUp);
    HodlStrategyConfig& setPercentDown(float percentDown);
    HodlStrategyConfig& setEmit(const common::Emit& emit);

    // ACCESSORS
    const InitStrategy& initStrategy() const;
    float               percentUp() const;
    float               percentDown() const;
    const common::Emit& emit() const;

}; // HodlStrategyConfig

struct Trade {
    // PUBLIC DATA
    // time when the trade was finalized
    std::string d_timestamp;
    // price at which the trade was executed
    float d_price;
    // did we already buy the dip? If yes don't do it again lol.
    bool d_boughtAgain;
}; // Trade

class HodlStrategy : public protocols::Strategy {

  private:
    // PRIVATE TYPES
    typedef std::unordered_map<unsigned int, Trade> TradeMap;

    struct BuyConfig {
        // Timestamp
        std::string_view d_timestamp;
        // Price we bought at.
        float d_price;
        // If the buy again flag should be set
        bool d_buyAgain;
    }; // BuyConfig

    struct SellConfig {
        // Position we are selling.
        TradeMap::iterator d_trade;
        // price we are selling at
        float d_price;
    }; // SellConfig

    // PRIVATE DATA
    // monotonically increasing trade id;
    unsigned int d_tradeIdBasis;
    // List of trades that have been finalized.
    TradeMap d_trades;
    // The price that helps us to descern what's the best course of action
    // to take when initially starting or we sold our last position.
    boost::optional<float> d_basisMarketPrice;
    // Config to the hodl strategy.
    HodlStrategyConfig d_config;

  public:
    // CREATORS
    HodlStrategy(const HodlStrategyConfig& config);
    ~HodlStrategy();

    // MANIPULATORS
    void handleNewData(const nlohmann::json& data) override;

    // ACCESSORS
    // Return a non-modifiable reference to the list of positions
    // we currently have
    const TradeMap& trades() const;
    // Return a non-modifiable reference to the basisMarketPrice.
    const boost::optional<float>& basisMarketPrice() const;

  private:
    // PRIVATE MANIPULATORS
    void goOverTradesAtPrice(float price, const std::string_view& timestamp);
    void buy(const BuyConfig& config);
    void sell(const SellConfig& config);

}; // HodlStrategy

// INLINE DEFINITIONS
// class HodlStrategyConfig

inline HodlStrategyConfig&
HodlStrategyConfig::setInitStrategy(const InitStrategy& initStrat)
{
    d_initStrategy = initStrat;
    return *this;
}

inline HodlStrategyConfig& HodlStrategyConfig::setPercentUp(float percentUp)
{
    d_percentUp = percentUp;
    return *this;
}

inline HodlStrategyConfig&
HodlStrategyConfig::setPercentDown(float percentDown)
{
    d_percentDown = percentDown;
    return *this;
}

inline HodlStrategyConfig&
HodlStrategyConfig::setEmit(const common::Emit& emit)
{
    d_emit = emit;
    return *this;
}

inline const HodlStrategyConfig::InitStrategy&
HodlStrategyConfig::initStrategy() const
{
    return d_initStrategy;
}

inline float HodlStrategyConfig::percentUp() const { return d_percentUp; }

inline float HodlStrategyConfig::percentDown() const { return d_percentDown; }

inline const common::Emit& HodlStrategyConfig::emit() const { return d_emit; }

// class HodlStrategy

// ACCESSORS
inline const HodlStrategy::TradeMap& HodlStrategy::trades() const
{
    return d_trades;
}

inline const boost::optional<float>& HodlStrategy::basisMarketPrice() const
{
    return d_basisMarketPrice;
}

} // namespace strategies
} // namespace crypto_trader

#endif // INCLUDED_HODL_STRATEGY
