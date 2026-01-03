#include "event_position_manager.h"

#include "../common/Accounting.h"
#include "../common/math.h"
#include "../common/timeutils.h"

#include <algorithm>
#include <optional>
#include <ranges>
#include <spdlog/spdlog.h>
#include <string_view>
#include <vector>

namespace crypto_trader {
namespace traders {

// CREATORS
EventPositionManager::EventPositionManager() {}
EventPositionManager::~EventPositionManager() {}

// MANIPULATORS

void EventPositionManager::register_observer(protocols::Observer *observer)
{
    d_observers.push_back(observer);
}

void EventPositionManager::unregister_observer(protocols::Observer *observer)
{
    auto it = std::find(d_observers.begin(), d_observers.end(), observer);
    if (it != d_observers.end()) {
        d_observers.erase(it);
    }
}

void EventPositionManager::clear(const std::string_view& symbol) {}

void EventPositionManager::clearAll() {}

void EventPositionManager::setEventsDb(const MEDP& db) { d_db_p = db; }

// PositionManager implementation
void EventPositionManager::submit_event(const common::Event& e)
{
    if (d_db_p) {
        d_db_p->logEvent(e);
    }

    d_accounting.apply_event(e);

    if (e.d_type == common::EventType::ORDER_FILLED) {
        common::Trade trade;
        trade.d_symbol   = e.d_symbol;
        trade.d_price    = e.d_price;
        trade.d_quantity = e.d_qty;

        for (auto observer : d_observers) {
            observer->on_trade(trade);
        }
    }

    auto snapshot = d_accounting.snapshot();
    auto it       = snapshot.find(e.d_symbol);
    if (it != snapshot.end()) {
        for (auto observer : d_observers) {
            observer->on_position_update(e.d_symbol, it->second.d_totalQty);
        }
    }

    auto now = common::getCurrentTimestampMs();

    if (now - d_lastSnapshotTime >= d_snapshotInterval) {
        takeSnapshot();
        d_lastSnapshotTime = now;
    }
}

std::optional<double>
EventPositionManager::currentHoldings(const std::string_view& symbol) const
{
    auto snapshot = d_accounting.snapshot();
    auto it       = snapshot.find(std::string(symbol));
    if (it != snapshot.end()) {
        return it->second.d_totalQty;
    }
    return std::nullopt;
}

std::optional<double>
EventPositionManager::averageCostBasis(const std::string_view& symbol) const
{
    auto snapshot = d_accounting.snapshot();
    auto it       = snapshot.find(std::string(symbol));
    if (it != snapshot.end() &&
        crypto_trader::common::Math::isGreater(it->second.d_totalQty, 0.0))
    {
        return it->second.d_averagePrice;
    }
    return std::nullopt;
}

std::optional<double>
EventPositionManager::realizedPnl(const std::string_view& symbol) const
{
    const auto& snapshot = d_accounting.snapshot();
    auto        it       = snapshot.find(std::string(symbol));

    if (it != snapshot.end()) {
        return it->second.d_realizedPnl;
    }
    return std::nullopt;
}
std::optional<double>
EventPositionManager::unrealizedPnl(const std::string_view& symbol,
                                    double                  currentPrice) const
{
    const auto& snapshot = d_accounting.snapshot();
    auto        it       = snapshot.find(std::string(symbol));

    if (it != snapshot.end()) {
        if (crypto_trader::common::Math::isGreater(it->second.d_totalQty, 0.0))
        {
            return (currentPrice - it->second.d_averagePrice) *
                   it->second.d_totalQty;
        }
    }
    return std::nullopt;
}
// PRIVATE METHODS
void EventPositionManager::takeSnapshot() const
{
    if (!d_db_p)
        return;

    const auto& snapshot = d_accounting.snapshot();

    if (snapshot.empty()) {
        SPDLOG_DEBUG("No positions to snapshot");
        return;
    }
    int64_t timestamp = common::getCurrentTimestampMs();

    std::vector<common::SymbolPositions> symbolPositions =
        snapshot | std::views::values | std::ranges::to<std::vector>();

    if (d_db_p->logSnapshots(symbolPositions)) {
        SPDLOG_DEBUG("Saved {} positions at timestamp {}",
                     symbolPositions.size(),
                     timestamp);
    }
    else {
        SPDLOG_ERROR("Failed to take positions snapshot");
    }
}

} // namespace traders
} // namespace crypto_trader
