#ifndef INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H
#define INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H

#include "Event.h"
#include <list>
#include <map>
#include <vector>

/**
 * @brief Represents a single position (lot) within a larger holding.
 */
struct Position {
    double  d_total_qty = 0.0; //!< Quantity of the asset in this lot.
    double  d_price     = 0.0; //!< Price at which this lot was acquired.
    int64_t d_timestamp;       //!< Timestamp when this lot was acquired.
};

/**
 * @brief Aggregates all positions and financial data for a specific trading symbol.
 */
struct SymbolPositions {
    std::string         d_symbol;          //!< The trading symbol (e.g., "BTC-USD").
    double              d_total_qty         = 0.0; //!< Total quantity of the asset held.
    double              d_average_price     = 0.0; //!< Weighted average price of all held lots.
    std::list<Position> d_positions_in_time = {}; //!< List of individual lots, ordered by time or FIFO/LIFO.
    bool                d_fifo              = true; //!< True if positions are managed using FIFO, false for LIFO.
    double              d_realizedPnl       = 0.0; //!< Realized Profit and Loss from closed positions.
};

/**
 * @brief Manages the accounting of trading activities, tracking positions and PnL.
 *
 * This class processes trade events to maintain an accurate record of holdings,
 * average prices, and realized profit/loss for various trading symbols.
 */
class Accounting {
  public:
    // TYPES
    using PositionMap = std::map<std::string, SymbolPositions>; //!< Map from symbol string to its aggregated positions.

    /**
     * @brief Applies a single trade event to update the accounting state.
     * @param e The trade event to apply.
     */
    void apply_event(const Event& e);

    /**
     * @brief Provides a const reference to the current snapshot of all positions.
     * @return A const reference to the map of symbol positions.
     */
    const PositionMap& snapshot() const;

    /**
     * @brief Replays a vector of historical events to reconstruct the accounting state.
     *        Clears all current positions and event logs before replaying.
     * @param events A vector of historical trade events.
     */
    void replay_events(const std::vector<Event>& events);

    // CREATORS
    /**
     * @brief Constructs an empty Accounting object.
     */
    Accounting() = default;

  private:
    std::vector<Event> d_event_log_; //!< Log of all processed events.
    PositionMap        d_positions_; //!< Map storing aggregated positions for each symbol.
};

#endif // INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H