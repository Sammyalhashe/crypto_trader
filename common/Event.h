#ifndef INCLUDED_CRYPTO_TRADER_COMMON_EVENT_H
#define INCLUDED_CRYPTO_TRADER_COMMON_EVENT_H

#include <nlohmann/json.hpp>
#include <string>

namespace crypto_trader {
namespace common {

using nlohmann::json;

enum class EventType {
    ORDER_SUBMITTED,
    ORDER_FILLED,
    ORDER_CANCELLED,
    POSITION_UPDATE
};

struct Event {
    std::string d_symbol;
    double      d_qty;
    double      d_price;
    EventType   d_type;
    json        d_payload;
    int64_t     d_timestamp;
};

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_COMMON_EVENT_H
