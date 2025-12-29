#include "Accounting.h"
#include "math.h"
#include <algorithm>
#include <spdlog/spdlog.h>

void Accounting::apply_event(const Event& e)
{
    d_event_log_.push_back(e);

    if (e.d_type == EventType::ORDER_FILLED) {
        auto& symbolPositions    = d_positions_[e.d_symbol];
        symbolPositions.d_symbol = e.d_symbol;
        if (crypto_trader::common::Math::isGreater(e.d_qty, 0.0)) // Buy
        {
            symbolPositions.d_positions_in_time.emplace_back();
            auto&  position = symbolPositions.d_positions_in_time.back();
            double current_total_value =
                symbolPositions.d_total_qty * symbolPositions.d_average_price;
            double new_order_value = e.d_qty * e.d_price;

            position.d_total_qty = e.d_qty;
            position.d_price     = e.d_price;
            position.d_timestamp = e.d_timestamp;

            symbolPositions.d_total_qty += e.d_qty;
            symbolPositions.d_average_price =
                (current_total_value + new_order_value) /
                symbolPositions.d_total_qty;
            symbolPositions.d_timestamp = e.d_timestamp;
        }
        else { // Sell
            double qty_to_sell = -e.d_qty;

            if (crypto_trader::common::Math::isLessOrEqual(
                    symbolPositions.d_total_qty, 0.0) ||
                crypto_trader::common::Math::isLess(
                    symbolPositions.d_total_qty, qty_to_sell))
            {
                SPDLOG_ERROR(
                    "Attempting to sell more {} than available.\nTotal "
                    "available quantity: {}\nAttempting to sell: {}",
                    symbolPositions.d_symbol,
                    symbolPositions.d_total_qty,
                    qty_to_sell);
                return;
            }

            while (crypto_trader::common::Math::isGreater(qty_to_sell, 0.0) &&
                   !symbolPositions.d_positions_in_time.empty())
            {
                auto& position =
                    symbolPositions.d_fifo
                        ? symbolPositions.d_positions_in_time.front()
                        : symbolPositions.d_positions_in_time.back();

                double quantity_to_sell =
                    std::min(qty_to_sell, position.d_total_qty);
                qty_to_sell -= quantity_to_sell;
                position.d_total_qty -= quantity_to_sell;

                symbolPositions.d_realizedPnl +=
                    (e.d_price - position.d_price) * quantity_to_sell;
                symbolPositions.d_total_qty -= quantity_to_sell;
                symbolPositions.d_timestamp = e.d_timestamp;

                bool shouldRemove = crypto_trader::common::Math::isLessOrEqual(
                    position.d_total_qty, 0.0);

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

            if (crypto_trader::common::Math::isGreater(
                    symbolPositions.d_total_qty, 0.0))
            {
                double totalCost = 0.0;
                for (const auto& lot : symbolPositions.d_positions_in_time) {
                    totalCost += lot.d_price * lot.d_total_qty;
                }
                symbolPositions.d_average_price =
                    totalCost / symbolPositions.d_total_qty;
            }
            else {
                symbolPositions.d_average_price = 0.0;
                symbolPositions.d_total_qty     = 0.0;
                symbolPositions.d_timestamp     = e.d_timestamp;
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
    d_event_log_.clear();
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
