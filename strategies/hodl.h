#ifndef INCLUDED_HODL_STRATEGY
#define INCLUDED_HODL_STRATEGY

#include "../common/types.h"
#include "../protocols/strategy.h"

#include <boost/beast/core.hpp>

#include <iostream>
#include <mach/host_info.h>
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
    // Callback called to see if we actually have funds to do a trade
    protocols::Strategy::FundsCb d_fundsCb;
    // Amount to buy each time.
    double d_buyAmount;
    double d_fundsAvailable;

  public:
    // MANIPULATORS
    HodlStrategyConfig& setInitStrategy(const InitStrategy& initStrat);
    HodlStrategyConfig& setPercentUp(float percentUp);
    HodlStrategyConfig& setPercentDown(float percentDown);
    HodlStrategyConfig& setEmit(const common::Emit& emit);
    HodlStrategyConfig&
    setFundsCb(const protocols::Strategy::FundsCb& fundsCb);
    HodlStrategyConfig& setBuyAmount(double buyAmount);
    HodlStrategyConfig& setFundsAvailable(double fundsAvailable);

    // ACCESSORS
    const InitStrategy&                 initStrategy() const;
    float                               percentUp() const;
    float                               percentDown() const;
    const common::Emit&                 emit() const;
    const protocols::Strategy::FundsCb& fundsCb() const;
    double                              buyAmount() const;
    double                              fundsAvailable() const;

}; // HodlStrategyConfig

struct Trade {
    // PUBLIC DATA
    // What was traded
    std::string d_symbol;
    // time when the trade was finalized
    std::string d_timestamp;
    // price at which the trade was executed
    double d_price;
    // did we already buy the dip? If yes don't do it again lol.
    bool d_boughtAgain;
}; // Trade

class HodlStrategy : public protocols::Strategy {

  private:
    // PRIVATE TYPES
    typedef std::string                             Symbol;
    typedef std::unordered_map<unsigned int, Trade> TradeMap;
    typedef std::unordered_map<Symbol, TradeMap>    TradesForSymbol;
    typedef std::unordered_map<Symbol, double>      LastPriceMap;

    struct BuyConfig {
        // What was traded
        std::string d_symbol;
        // Timestamp
        std::string_view d_timestamp;
        // Price we bought at.
        double d_price;
        // If the buy again flag should be set
        bool d_buyAgain;
    }; // BuyConfig

    struct SellConfig {
        // What was traded
        std::string d_symbol;
        // Position we are selling.
        TradeMap::iterator d_trade;
        // price we are selling at
        double d_price;
    }; // SellConfig

    // PRIVATE DATA
    // monotonically increasing trade id;
    unsigned int d_tradeIdBasis;
    // List of trades that have been finalized.
    TradesForSymbol d_trades;
    // The price that helps us to descern what's the best course of action
    // to take when initially starting or we sold our last position.
    boost::optional<double> d_basisMarketPrice;
    // TODO Remove this and query for prices at the time.
    // Map of the last recorded price
    LastPriceMap d_symbolToLastPrice;
    // Config to the hodl strategy.
    HodlStrategyConfig d_config;

  public:
    // CREATORS
    HodlStrategy(const HodlStrategyConfig& config);
    ~HodlStrategy();

    // MANIPULATORS
    // protocols::strategy
    void handleNewData(const nlohmann::json& data) override;

    // ACCESSORS
    // Return a non-modifiable reference to the list of positions
    // we currently have
    const TradesForSymbol& trades() const;
    // Return a non-modifiable reference to the basisMarketPrice.
    const boost::optional<double>& basisMarketPrice() const;
    // protocols::strategy
    double totalPosition() const override;

  private:
    // PRIVATE MANIPULATORS
    void goOverTradesAtPrice(const std::string& symbol,
                             double             price,
                             const std::string& timestamp);
    void buy(const BuyConfig& config);
    void sell(const SellConfig& config);

    // FRIENDS
    friend HodlStrategyConfig;

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

inline HodlStrategyConfig&
HodlStrategyConfig::setFundsCb(const protocols::Strategy::FundsCb& fundsCb)
{
    d_fundsCb = fundsCb;
    return *this;
}

inline HodlStrategyConfig& HodlStrategyConfig::setBuyAmount(double buyAmount)
{
    d_buyAmount = buyAmount;
    return *this;
}

inline HodlStrategyConfig&
HodlStrategyConfig::setFundsAvailable(double fundsAvailable)
{
    d_fundsAvailable = fundsAvailable;
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

inline const protocols::Strategy::FundsCb& HodlStrategyConfig::fundsCb() const
{
    return d_fundsCb;
}

inline double HodlStrategyConfig::buyAmount() const { return d_buyAmount; }

inline double HodlStrategyConfig::fundsAvailable() const
{
    return d_fundsAvailable;
}

// class HodlStrategy

// ACCESSORS
inline const HodlStrategy::TradesForSymbol& HodlStrategy::trades() const
{
    return d_trades;
}

inline const boost::optional<double>& HodlStrategy::basisMarketPrice() const
{
    return d_basisMarketPrice;
}

} // namespace strategies
} // namespace crypto_trader

#endif // INCLUDED_HODL_STRATEGY
