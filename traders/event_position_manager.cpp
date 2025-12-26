#include "event_position_manager.h"
#include "../common/math.h"

#include <algorithm>
#include <optional>
#include <spdlog/spdlog.h>
#include <string_view>

namespace crypto_trader {
namespace traders {

void EventPositionManager::submit_event(const Event& e)
{
    if (d_db_p) {
        d_db_p->logEvent(e);
    }

    d_accounting_.apply_event(e);

    if (e.d_type == EventType::ORDER_FILLED) {
        common::Trade trade;
        trade.d_symbol   = e.d_symbol;
        trade.d_price    = e.d_price;
        trade.d_quantity = e.d_qty;

        for (auto observer : d_observers) {
            observer->on_trade(trade);
        }
    }

    auto snapshot = d_accounting_.snapshot();
    auto it       = snapshot.find(e.d_symbol);
    if (it != snapshot.end()) {
        for (auto observer : d_observers) {
            observer->on_position_update(e.d_symbol, it->second.d_total_qty);
        }
    }
}

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

std::optional<double>
EventPositionManager::currentHoldings(const std::string_view& symbol) const
{
    auto snapshot = d_accounting_.snapshot();
    auto it       = snapshot.find(std::string(symbol));
    if (it != snapshot.end()) {
        return it->second.d_total_qty;
    }
    return std::nullopt;
}

std::optional<double>
EventPositionManager::averageCostBasis(const std::string_view& symbol) const
{
    auto snapshot = d_accounting_.snapshot();
    auto it       = snapshot.find(std::string(symbol));
    if (it != snapshot.end() &&
        crypto_trader::common::Math::isGreater(it->second.d_total_qty, 0.0))
    {
        return it->second.d_average_price;
    }
    return std::nullopt;
}

std::optional<double>
EventPositionManager::realizedPnl(const std::string_view& symbol) const
{
    const auto& snapshot = d_accounting_.snapshot();
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
    const auto& snapshot = d_accounting_.snapshot();
    auto        it       = snapshot.find(std::string(symbol));

    if (it != snapshot.end()) {
        if (crypto_trader::common::Math::isGreater(it->second.d_total_qty,
                                                   0.0))
        {
            return (currentPrice - it->second.d_average_price) *
                   it->second.d_total_qty;
        }
    }
    return std::nullopt;
}

void EventPositionManager::clear(const std::string_view& symbol) {}

void EventPositionManager::clearAll() {}

void EventPositionManager::setEventsDb(const MEDP& db) { d_db_p = db; }

} // namespace traders
} // namespace crypto_trader
