#include "Accounting.h"
#include "math.h"
#include <algorithm>
#include <spdlog/spdlog.h>

#include "Accounting.h"
#include "math.h"
#include <algorithm>
#include <spdlog/spdlog.h>

namespace crypto_trader {
namespace common {

void Accounting::apply_event(const Event& e)
{
    d_eventLog.push_back(e);

    if (e.d_type == EventType::ORDER_FILLED) {
        auto& symbolPositions    = d_positions_[e.d_symbol];
        symbolPositions.d_symbol = e.d_symbol;
        if (Math::isGreater(e.d_qty, 0.0)) // Buy
        {
            symbolPositions.d_positions_in_time.emplace_back();
            auto&  position = symbolPositions.d_positions_in_time.back();
            double current_total_value =
                symbolPositions.d_totalQty * symbolPositions.d_averagePrice;
            double new_order_value = e.d_qty * e.d_price;

            position.d_totalQty  = e.d_qty;
            position.d_price     = e.d_price;
            position.d_timestamp = e.d_timestamp;

            symbolPositions.d_totalQty += e.d_qty;
            symbolPositions.d_averagePrice =
                (current_total_value + new_order_value) /
                symbolPositions.d_totalQty;
            symbolPositions.d_timestamp = e.d_timestamp;
        }
        else { // Sell
            double qty_to_sell = -e.d_qty;

            if (Math::isLessOrEqual(symbolPositions.d_totalQty, 0.0) ||
                Math::isLess(symbolPositions.d_totalQty, qty_to_sell))
            {
                SPDLOG_ERROR(
                    "Attempting to sell more {} than available.\nTotal "
                    "available quantity: {}\nAttempting to sell: {}",
                    symbolPositions.d_symbol,
                    symbolPositions.d_totalQty,
                    qty_to_sell);
                return;
            }

            while (Math::isGreater(qty_to_sell, 0.0) &&
                   !symbolPositions.d_positions_in_time.empty())
            {
                auto& position =
                    symbolPositions.d_fifo
                        ? symbolPositions.d_positions_in_time.front()
                        : symbolPositions.d_positions_in_time.back();

                double quantity_to_sell =
                    std::min(qty_to_sell, position.d_totalQty);
                qty_to_sell -= quantity_to_sell;
                position.d_totalQty -= quantity_to_sell;

                symbolPositions.d_realizedPnl +=
                    (e.d_price - position.d_price) * quantity_to_sell;
                symbolPositions.d_totalQty -= quantity_to_sell;
                symbolPositions.d_timestamp = e.d_timestamp;

                bool shouldRemove =
                    Math::isLessOrEqual(position.d_totalQty, 0.0);

                // remove position if fully sold
                if (shouldRemove) {
                    if (symbolPositions.d_fifo) {
                        symbolPositions.d_positions_in_time.pop_front();
                    }
                    else {
                        symbolPositions.d_positions_in_time.pop_back();
                    }
                }
            }

            if (Math::isGreater(symbolPositions.d_totalQty, 0.0)) {
                double totalCost = 0.0;
                for (const auto& lot : symbolPositions.d_positions_in_time) {
                    totalCost += lot.d_price * lot.d_totalQty;
                }
                symbolPositions.d_averagePrice =
                    totalCost / symbolPositions.d_totalQty;
            }
            else {
                symbolPositions.d_averagePrice = 0.0;
                symbolPositions.d_totalQty     = 0.0;
                symbolPositions.d_timestamp    = e.d_timestamp;
            }
        }
    }
}

const Accounting::PositionMap& Accounting::snapshot() const
{
    return d_positions_;
}

void Accounting::replay_events(const std::vector<Event>& events)
{
    d_positions_.clear();
    d_eventLog.clear();
    auto sorted_events = events;
    std::sort(sorted_events.begin(),
              sorted_events.end(),
              [](const Event& a, const Event& b) {
                  return a.d_timestamp < b.d_timestamp;
              });
    for (const auto& e : sorted_events) {
        apply_event(e);
    }
}

} // namespace common
} // namespace crypto_trader
