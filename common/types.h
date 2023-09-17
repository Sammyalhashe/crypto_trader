#ifndef INCLUDED_TYPES
#define INCLUDED_TYPES

#include <functional>
#include <string>

namespace crypto_trader {

namespace common {

struct Action {
    enum ActionType {
        e_BUY = 0,
        e_SELL = 1
    };

    // DATA
    // Type of the emitted action.
    ActionType d_type;
}; // struct Action

typedef std::function<void(const Action&)> Emit;

struct MarketDataCoinbase {
    using Timestamp = unsigned long long;
    std::string symbol;
    double price;
    Timestamp sequence;

    static struct {
        bool operator() (const MarketDataCoinbase& lhs, const MarketDataCoinbase& rhs) {
            return lhs.sequence < rhs.sequence;
        }

        bool operator() (const Timestamp& lhs, const MarketDataCoinbase& rhs) {
            return lhs < rhs.sequence;
        }

        bool operator() (const MarketDataCoinbase& lhs, const Timestamp& rhs) {
            return lhs.sequence < rhs;
        }
    } order;
};

} // common
} // crypto_trader


#endif // INCLUDED_TYPES
