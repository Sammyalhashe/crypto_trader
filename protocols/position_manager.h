/**
 * @file position_manager.h
 * @brief Abstract interface for managing trading positions in the
 * crypto_trader system.
 *
 * This file defines the PositionManager class, which provides an interface for
 * tracking and managing trading positions, including holdings, cost basis, and
 * profit/loss calculations for different symbols. Implementations of this
 * interface are responsible for handling trading events and maintaining
 * position state.
 */

#ifndef INCLUDED_CRYPTO_TRADER_PROTOCOLS_POSITION_MANAGER_H
#define INCLUDED_CRYPTO_TRADER_PROTOCOLS_POSITION_MANAGER_H

#include "../common/Event.h"
#include "../common/types.h"

#include <optional>
#include <string_view>

namespace crypto_trader {
namespace protocols {

class PositionManager {

  public:
    // CREATORS
    PositionManager()          = default;
    virtual ~PositionManager() = 0;

    // MANIPULATORS
    virtual void submit_event(const common::Event& e) = 0;

    // ACCESSORS
    // Get the current total holdings for a specific symbol.
    virtual std::optional<double>
    currentHoldings(const std::string_view& symbol) const = 0;

    // Get the average cost basis of the current holdings for a specific
    // symbol. Returns std::nullopt if there are no holdings for the symbol.
    virtual std::optional<double>
    averageCostBasis(const std::string_view& symbol) const = 0;

    // Get the total realized Profit and Loss for a specific symbol.
    virtual std::optional<double>
    realizedPnl(const std::string_view& symbol) const                      = 0;
    virtual std::optional<double> unrealizedPnl(const std::string_view& symbol,
                                                double currentPrice) const = 0;

}; // class PositionManager

} // namespace protocols
} // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_PROTOCOLS_POSITION_MANAGER_H
