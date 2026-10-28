#pragma once

#include "../common/types.h"

#include <optional>
#include <string_view>

namespace crypto_trader {
namespace protocols {

/**
 * @brief Abstract base class defining the interface for trade execution.
 *
 * @tparam T The MarketData type that the executor processes.
 */
template <common::MarketData T>
class Executor {
  public:
    /**
     * @brief Destructor for the Executor interface.
     */
    virtual ~Executor() = default;

    /**
     * @brief Executes a buy order for a specified product and quantity.
     * @param product The symbol of the product to buy (e.g., "BTC-USD").
     * @param quantity The amount of the product to buy.
     * @return The result of the trade execution.
     */
    virtual common::TradeResult buy(const std::string_view& product,
                                    double                  quantity) = 0;

    /**
     * @brief Executes a sell order for a specified product and quantity.
     * @param product The symbol of the product to sell (e.g., "BTC-USD").
     * @param quantity The amount of the product to sell.
     * @return The result of the trade execution.
     */
    virtual common::TradeResult sell(const std::string_view& product,
                                     double                  quantity) = 0;

    /**
     * @brief Retrieves the current balance of a specified currency.
     * @param currency The symbol of the currency (e.g., "USD", "BTC").
     * @return The current balance of the currency.
     */
    virtual double getBalance(const std::string_view& currency) const = 0;

    /**
     * @brief Retrieves the current position (holdings) for a given product.
     * @param product The symbol of the product (e.g., "BTC-USD").
     * @return An optional containing the quantity held, or empty if no
     * position.
     */
    virtual std::optional<double>
    getPosition(const std::string_view& product) const = 0;

    /**
     * @brief Processes new ticker data to update the executor's internal
     * state.
     * @param product The symbol of the product for the ticker.
     * @param price The latest price of the product.
     * @param timestamp The timestamp of the ticker data.
     */
    virtual void processTickerData(const std::string_view& product,
                                   double                  price,
                                   const T::Timestamp&     timestamp) = 0;
};

} // namespace protocols
} // namespace crypto_trader