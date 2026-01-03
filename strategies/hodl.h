#ifndef INCLUDED_HODL_STRATEGY
#define INCLUDED_HODL_STRATEGY

#include "../common/types.h"
#include "../protocols/observer.h"
#include "../protocols/strategy.h"
#include "../traders/event_position_manager.h"

#include <boost/beast/core.hpp>

#include <iostream>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace crypto_trader {
namespace strategies {

/**
 * @brief Configuration parameters for the HodlStrategy.
 */
class HodlStrategyConfig {
  public:
    /**
     * @brief Defines the initial strategy behavior.
     */
    enum InitStrategy {
        e_BUY_IMMEDIATELY = 0, //!< Buy immediately when the strategy starts.
        e_SET_BASIS_PRICE = 1 //!< Set current price as basis and wait for dip.
    }; // InitStrategy

  private:
    InitStrategy
          d_initStrategy; //!< Action to take when the strategy first starts.
    float d_percentUp;   //!< Percentage increase to trigger a sell for profit.
    float d_percentDown; //!< Percentage decrease to trigger a buy if the
                         //!< market dips.
    common::Emit
           d_emit; //!< Callback function used by the strategy to emit actions.
    double d_buyAmount; //!< Amount in USD (or base currency) to use for buy
                        //!< orders.

  public:
    /**
     * @brief Sets the initial strategy behavior.
     * @param initStrat The initial strategy to set.
     * @return A reference to the updated configuration object.
     */
    HodlStrategyConfig& setInitStrategy(const InitStrategy& initStrat);

    /**
     * @brief Sets the percentage increase for selling.
     * @param percentUp The percentage (e.g., 5.0 for 5%).
     * @return A reference to the updated configuration object.
     */
    HodlStrategyConfig& setPercentUp(float percentUp);

    /**
     * @brief Sets the percentage decrease for buying.
     * @param percentDown The percentage (e.g., 5.0 for 5%).
     * @return A reference to the updated configuration object.
     */
    HodlStrategyConfig& setPercentDown(float percentDown);

    /**
     * @brief Sets the emission callback function.
     * @param emit The callback function for emitting actions.
     * @return A reference to the updated configuration object.
     */
    HodlStrategyConfig& setEmit(const common::Emit& emit);

    /**
     * @brief Sets the amount to use for buy orders.
     * @param buyAmount The buy amount in base currency.
     * @return A reference to the updated configuration object.
     */
    HodlStrategyConfig& setBuyAmount(double buyAmount);

    /**
     * @brief Gets the initial strategy behavior.
     * @return The initial strategy.
     */
    const InitStrategy& initStrategy() const;

    /**
     * @brief Gets the percentage increase for selling.
     * @return The percentage up.
     */
    float percentUp() const;

    /**
     * @brief Gets the percentage decrease for buying.
     * @return The percentage down.
     */
    float percentDown() const;

    /**
     * @brief Gets the emission callback function.
     * @return The emission callback.
     */
    const common::Emit& emit() const;

    /**
     * @brief Gets the amount to use for buy orders.
     * @return The buy amount.
     */
    double buyAmount() const;

}; // HodlStrategyConfig

/**
 * @brief Implements a "Buy and Hold" strategy with dynamic adjustments based
 * on price movements.
 *
 * This strategy aims to buy on dips and potentially sell on significant gains.
 */
class HodlStrategy : public protocols::Strategy, public protocols::Observer {

  private:
    /**
     * @brief Minimal state maintained for each trading symbol.
     */
    struct SymbolState {
        double lastBuyPrice =
            0.0; //!< The price of the last buy order for this symbol.
        bool hasBoughtAgain = false; //!< True if a buy has occurred after a
                                     //!< previous sell signal.
        bool waitingForSell =
            false; //!< True if the strategy is currently holding and waiting
                   //!< for a sell condition.
    };

    std::unordered_map<std::string, SymbolState>
        d_symbolStates; //!< Map of symbol states.
    traders::EventPositionManager&
        d_positionManager; //!< Reference to the position manager.
    HodlStrategyConfig
        d_config; //!< Configuration for this HodlStrategy instance.

  public:
    /**
     * @brief Constructs a HodlStrategy.
     * @param config The configuration for the strategy.
     * @param positionManager A reference to the EventPositionManager for
     * tracking positions.
     */
    HodlStrategy(const HodlStrategyConfig&      config,
                 traders::EventPositionManager& positionManager);

    /**
     * @brief Destructor for HodlStrategy.
     */
    ~HodlStrategy();

    /**
     * @brief Handles new market data received by the strategy.
     * @param data The new market data, typically in JSON format.
     */
    void handleNewData(const nlohmann::json& data) override;

    /**
     * @brief Callback method invoked when a trade is executed.
     * @param trade The details of the executed trade.
     */
    void on_trade(const common::Trade& trade) override;

    /**
     * @brief Callback method invoked when a position update occurs.
     * @param symbol The trading symbol.
     * @param new_position The updated position quantity for the symbol.
     */
    void on_position_update(const std::string& symbol,
                            double             new_position) override;

  private:
    /**
     * @brief Internal helper to evaluate trade conditions at a given price.
     * @param product The trading product.
     * @param price The current market price.
     * @param timestamp The timestamp of the market data.
     */
    void goOverTradesAtPrice(const std::string_view& product,
                             double                  price,
                             const std::string_view& timestamp);

    /**
     * @brief Emits a buy action.
     * @param product The trading product.
     * @param price The price at which the buy is considered.
     * @param timestamp The timestamp of the buy action.
     */
    void buy(const std::string& product,
             double             price,
             const std::string& timestamp);

    /**
     * @brief Emits a sell action.
     * @param product The trading product.
     * @param price The price at which the sell is considered.
     * @param timestamp The timestamp of the sell action.
     */
    void sell(const std::string& product,
              double             price,
              const std::string& timestamp);

}; // HodlStrategy

// INLINE DEFINITIONS
// class HodlStrategyConfig

/**
 * @brief Sets the initial strategy behavior.
 * @param initStrat The initial strategy to set.
 * @return A reference to the updated configuration object.
 */
inline HodlStrategyConfig&
HodlStrategyConfig::setInitStrategy(const InitStrategy& initStrat)
{
    d_initStrategy = initStrat;
    return *this;
}

/**
 * @brief Sets the percentage increase for selling.
 * @param percentUp The percentage (e.g., 5.0 for 5%).
 * @return A reference to the updated configuration object.
 */
inline HodlStrategyConfig& HodlStrategyConfig::setPercentUp(float percentUp)
{
    d_percentUp = percentUp;
    return *this;
}

/**
 * @brief Sets the percentage decrease for buying.
 * @param percentDown The percentage (e.g., 5.0 for 5%).
 * @return A reference to the updated configuration object.
 */
inline HodlStrategyConfig&
HodlStrategyConfig::setPercentDown(float percentDown)
{
    d_percentDown = percentDown;
    return *this;
}

/**
 * @brief Sets the emission callback function.
 * @param emit The callback function for emitting actions.
 * @return A reference to the updated configuration object.
 */
inline HodlStrategyConfig&
HodlStrategyConfig::setEmit(const common::Emit& emit)
{
    d_emit = emit;
    return *this;
}

/**
 * @brief Sets the amount to use for buy orders.
 * @param buyAmount The buy amount in base currency.
 * @return A reference to the updated configuration object.
 */
inline HodlStrategyConfig& HodlStrategyConfig::setBuyAmount(double buyAmount)
{
    d_buyAmount = buyAmount;
    return *this;
}

/**
 * @brief Gets the initial strategy behavior.
 * @return The initial strategy.
 */
inline const HodlStrategyConfig::InitStrategy&
HodlStrategyConfig::initStrategy() const
{
    return d_initStrategy;
}

/**
 * @brief Gets the percentage increase for selling.
 * @return The percentage up.
 */
inline float HodlStrategyConfig::percentUp() const { return d_percentUp; }

/**
 * @brief Gets the percentage decrease for buying.
 * @return The percentage down.
 */
inline float HodlStrategyConfig::percentDown() const { return d_percentDown; }

/**
 * @brief Gets the emission callback function.
 * @return The emission callback.
 */
inline const common::Emit& HodlStrategyConfig::emit() const { return d_emit; }

/**
 * @brief Gets the amount to use for buy orders.
 * @return The buy amount.
 */
inline double HodlStrategyConfig::buyAmount() const { return d_buyAmount; }

} // namespace strategies
} // namespace crypto_trader

#endif // INCLUDED_HODL_STRATEGY