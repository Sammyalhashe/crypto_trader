#ifndef INCLUDED_CRYPTO_TRADER_TRADERS_EVENT_POSITION_MANAGER_H
#define INCLUDED_CRYPTO_TRADER_TRADERS_EVENT_POSITION_MANAGER_H

#include "../common/Accounting.h"
#include "../common/Event.h"
#include "../databases/market_events_db.h"
#include "../protocols/observer.h"
#include <optional>
#include <string_view>
#include <vector>

namespace crypto_trader {
namespace traders {

class EventPositionManager {
  private:
    // TYPES
    using MEDP = databases::MarketEventsDb::MarketEventsDbPtr;
    // DATA
    Accounting                         d_accounting_;
    std::vector<protocols::Observer *> d_observers;
    MEDP                               d_db_p;

  public:
    EventPositionManager() = default;

    // INFO: Why is this virtual?
    virtual void submit_event(const Event& e);

    void register_observer(protocols::Observer *observer);
    void unregister_observer(protocols::Observer *observer);

    // Get the current total holdings for a specific symbol.
    std::optional<double>
    currentHoldings(const std::string_view& symbol) const;

    // Get the average cost basis of the current holdings for a specific
    // symbol. Returns std::nullopt if there are no holdings for the symbol.
    std::optional<double>
    averageCostBasis(const std::string_view& symbol) const;

    // Get the total realized Profit and Loss for a specific symbol.
    std::optional<double> realizedPnl(const std::string_view& symbol) const;
    std::optional<double> unrealizedPnl(const std::string_view& symbol,
                                        double currentPrice) const;

    // Clear all positions and reset PnL for a specific symbol.
    void clear(const std::string_view& symbol);

    // Clear all positions and reset PnL for all symbols.
    void clearAll();

    void setEventsDb(const MEDP& db);
};

} // namespace traders
} // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_TRADERS_EVENT_POSITION_MANAGER_H
