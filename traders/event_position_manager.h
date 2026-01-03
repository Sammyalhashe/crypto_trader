/**
 * @file event_position_manager.h
 * @brief Concrete implementation of PositionManager for event-driven position
 * tracking.
 *
 * This file defines the EventPositionManager class, which implements the
 * PositionManager interface using an event-driven approach. It manages trading
 * positions, observers, and interacts with a market events database to track
 * holdings, cost basis, and profit/loss for different symbols. The class also
 * supports observer registration, clearing positions, and periodic state
 * snapshots.
 */

#ifndef INCLUDED_CRYPTO_TRADER_TRADERS_EVENT_POSITION_MANAGER_H
#define INCLUDED_CRYPTO_TRADER_TRADERS_EVENT_POSITION_MANAGER_H

#include "../common/Accounting.h"
#include "../common/Event.h"
#include "../databases/market_events_db.h"
#include "../protocols/observer.h"
#include "../protocols/position_manager.h"

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace crypto_trader {
namespace traders {

class EventPositionManager : public protocols::PositionManager {
  private:
    // TYPES
    using MEDP = databases::MarketEventsDb::MarketEventsDbPtr;
    // DATA
    common::Accounting                 d_accounting;
    std::vector<protocols::Observer *> d_observers;
    MEDP                               d_db_p;
    int64_t                            d_lastSnapshotTime{0};

    int64_t d_snapshotInterval{60000};

  public:
    EventPositionManager();
    ~EventPositionManager() override;

    // MANIPULATORS

    void register_observer(protocols::Observer *observer);

    void unregister_observer(protocols::Observer *observer);

    // Clear all positions and reset PnL for a specific symbol.
    void clear(const std::string_view& symbol);

    // Clear all positions and reset PnL for all symbols.
    void clearAll();

    void setEventsDb(const MEDP& db);

    // PositionManager implementation

    virtual void submit_event(const common::Event& e) override;

    // Get the current total holdings for a specific symbol.
    std::optional<double>
    currentHoldings(const std::string_view& symbol) const override;

    // Get the average cost basis of the current holdings for a specific
    // symbol. Returns std::nullopt if there are no holdings for the symbol.
    std::optional<double>
    averageCostBasis(const std::string_view& symbol) const override;

    // Get the total realized Profit and Loss for a specific symbol.
    std::optional<double>
    realizedPnl(const std::string_view& symbol) const override;
    std::optional<double> unrealizedPnl(const std::string_view& symbol,
                                        double currentPrice) const override;

  private:
    // PRIVATE METHODS
    /**
     * @brief takes a snapshot from the internal accounting class and saves it
     * in the database
     */
    void takeSnapshot() const;
};

} // namespace traders
} // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_TRADERS_EVENT_POSITION_MANAGER_H
