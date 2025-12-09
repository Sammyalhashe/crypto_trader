#ifndef INCLUDED_TYPES
#define INCLUDED_TYPES

#include "serialization.h"

#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

namespace crypto_trader {

namespace common {

enum class Side { e_BUY = 0, e_SELL = 1 };

inline std::ostream& operator<<(std::ostream& os, Side s)
{
    switch (s) {
    case Side::e_BUY:
        os << "BUY";
        break;
    case Side::e_SELL:
        os << "SELL";
        break;
    default:
        break;
    }
    return os;
}

struct TradeResult {
    bool        d_success;
    double      d_fillPrice;
    double      d_commission;
    std::string d_errorMessage;
};

struct Action {
    // DATA
    // Type of the emitted action.
    Side d_type;
    // Product for the action.
    std::string d_product;
    // Quantity for the action.
    double d_quantity;
}; // struct Action

struct Trade {
    std::string d_symbol;
    double d_price;
    double d_quantity;
};

typedef std::function<void(const Action&)> Emit;

template <typename T>
concept Serializeable = requires(T a) {
    // requires an `order` static member.
    T::order;
    // requires a `serialize` method.
    { a.serialize() };
}; // concept Serializeable

// Requires the type to have a `Timestamp` type internal
template <typename T>
concept MarketData = requires(T a) {
    // requires type `Timestamp`.
    typename T::Timestamp;
}; // concept MarketData

template <typename T>
concept SerializeableData = requires(T a) {
    requires MarketData<T>;
    requires Serializeable<T>;
};

template <typename T>
concept TimestampLike = requires(T t) {
    // Must be convertible to int64_t
    { static_cast<int64_t>(t) } -> std::convertible_to<int64_t>;

    // Must support comparison operators
    { t < t } -> std::convertible_to<bool>;
    { t == t } -> std::convertible_to<bool>;

    // Should be trivially copyable for performance
} && std::is_trivially_copyable_v<T>;

class MarketDataCoinbase {
  public:
    // PUBLIC TYPES
    using Timestamp = int64_t;
    // STATIC MEMBERS
    static struct {
        bool operator()(const MarketDataCoinbase& lhs,
                        const MarketDataCoinbase& rhs)
        {
            return lhs.d_sequence < rhs.d_sequence;
        }

        bool operator()(const Timestamp& lhs, const MarketDataCoinbase& rhs)
        {
            return lhs < rhs.d_sequence;
        }

        bool operator()(const MarketDataCoinbase& lhs, const Timestamp& rhs)
        {
            return lhs.d_sequence < rhs;
        }
    } order;
    // PUBLIC DATA
    std::string d_symbol;
    double      d_price;
    int64_t     d_sequence;
    Timestamp   d_timestamp;

  private:
    // PRIVATE MANIPULATORS
    template <class Archive>
    void serialize(Archive& archive, const unsigned int version);

    // FRIENDS
    friend class boost::serialization::access;
}; // MarketDataCoinbase

// class MarketDataCoinbase

// PRIVATE MANIPULATORS
template <class Archive>
void MarketDataCoinbase::serialize(Archive&           archive,
                                   const unsigned int version)
{
    archive & d_symbol;
    archive & d_price;
    archive & d_sequence;
    archive & d_timestamp;
}

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_TYPES
