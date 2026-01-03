#ifndef INCLUDED_TYPES
#define INCLUDED_TYPES

#include "serialization.h"

#include <cstdint>
#include <fmt/core.h>
#include <functional>
#include <ostream>
#include <string>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

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
    double      d_price;
    double      d_quantity;
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

/**
 * @brief fmt formatter specialization for crypto_trader::common::Side.
 *
 * Allows formatting of Side enum values using fmt library.
 * Outputs "BUY" for e_BUY, "SELL" for e_SELL, and "UNKNOWN" for other values.
 */
template <>
struct fmt::formatter<crypto_trader::common::Side> {
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(crypto_trader::common::Side s, FormatContext& ctx) const
    {
        switch (s) {
        case crypto_trader::common::Side::e_BUY:
            return fmt::format_to(ctx.out(), "BUY");
        case crypto_trader::common::Side::e_SELL:
            return fmt::format_to(ctx.out(), "SELL");
        default:
            return fmt::format_to(ctx.out(), "UNKNOWN");
        }
    }
};

/**
 * @brief fmt formatter specialization for crypto_trader::common::Action.
 *
 * Enables custom formatting of Action objects using the fmt library.
 * Formats as: "{Side}({product}, {quantity})", with quantity shown to two decimal places.
 */
template <>
struct fmt::formatter<crypto_trader::common::Action> {
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const crypto_trader::common::Action& act,
                FormatContext&                       ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "{}({}, {:.2f})",
                              act.d_type,
                              act.d_product,
                              act.d_quantity);
    }
};

#endif // INCLUDED_TYPES
