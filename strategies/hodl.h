#ifndef INCLUDED_HODL_STRATEGY
#define INCLUDED_HODL_STRATEGY

#include "../common/types.h"
#include "../protocols/strategy.h"
#include "../protocols/observer.h"
#include "../traders/event_position_manager.h"

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
    // amount in USD to buy
    double d_buyAmount;

  public:
    // MANIPULATORS
    HodlStrategyConfig& setInitStrategy(const InitStrategy& initStrat);
    HodlStrategyConfig& setPercentUp(float percentUp);
    HodlStrategyConfig& setPercentDown(float percentDown);
    HodlStrategyConfig& setEmit(const common::Emit& emit);
    HodlStrategyConfig& setBuyAmount(double buyAmount);

    // ACCESSORS
    const InitStrategy& initStrategy() const;
    float               percentUp() const;
    float               percentDown() const;
    const common::Emit& emit() const;
    double              buyAmount() const;

}; // HodlStrategyConfig

class HodlStrategy : public protocols::Strategy, public protocols::Observer {

  private:
    // MINIMAL derived state per symbol
    struct SymbolState {
        double lastBuyPrice = 0.0;
        bool hasBoughtAgain = false;
        bool waitingForSell = false;
    };
    
    std::unordered_map<std::string, SymbolState> d_symbolStates;
    traders::EventPositionManager& d_positionManager;
    // Config to the hodl strategy.
    HodlStrategyConfig d_config;

  public:
    // CREATORS
    HodlStrategy(const HodlStrategyConfig& config,
                 traders::EventPositionManager& positionManager);
    ~HodlStrategy();

    // MANIPULATORS
    void handleNewData(const nlohmann::json& data) override;
    void on_trade(const common::Trade& trade) override;
    void on_position_update(const std::string& symbol, double new_position) override;

  private:
    // PRIVATE MANIPULATORS
    void goOverTradesAtPrice(const std::string_view& product,
                             double                  price,
                             const std::string_view& timestamp);
    void buy(const std::string& product, double price, const std::string& timestamp);
    void sell(const std::string& product, double price, const std::string& timestamp);

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

inline HodlStrategyConfig& HodlStrategyConfig::setBuyAmount(double buyAmount)
{
    d_buyAmount = buyAmount;
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

inline double HodlStrategyConfig::buyAmount() const { return d_buyAmount; }

} // namespace strategies
} // namespace crypto_trader

#endif // INCLUDED_HODL_STRATEGY
