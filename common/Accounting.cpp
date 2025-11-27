#include "Accounting.h"

void Accounting::apply_event(const Event& e)
{
    d_event_log_.push_back(e);

    if (e.d_type == EventType::ORDER_FILLED) {
        auto& position = d_positions_[e.d_symbol];
        if (e.d_qty > 0) // Buy
        {
            double current_total_value =
                position.d_total_qty * position.d_average_price;
            double new_order_value = e.d_qty * e.d_price;

            position.d_total_qty += e.d_qty;

            // FIX: This check is redundant I think
            if (position.d_total_qty != 0) {
                position.d_average_price =
                    (current_total_value + new_order_value) /
                    position.d_total_qty;
            }
            else {
                position.d_average_price = 0;
            }
        }
        else { // Sell
            position.d_total_qty += e.d_qty;

            // FIX: So what I can gather is that this currently allows
            // for submitting an order for more quantity than you own
            if (position.d_total_qty <= 0) {
                position.d_average_price = 0;
                position.d_total_qty     = 0;
            }
        }
    }
}

const std::map<std::string, Position>& Accounting::snapshot() const
{
    return d_positions_;
}

void Accounting::replay_events(const std::vector<Event>& events)
{
    d_positions_.clear();
    for (const auto& e : events) {
        apply_event(e);
    }
}
