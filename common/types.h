#ifndef INCLUDED_TYPES
#define INCLUDED_TYPES

#include "serialization.h"

#include <functional>
#include <string>

namespace crypto_trader {

namespace common {

struct Action {
    enum ActionType { e_BUY = 0, e_SELL = 1 };

    // DATA
    // Type of the emitted action.
    ActionType d_type;
}; // struct Action

typedef std::function<void(const Action&)> Emit;

template <typename T>
concept Serializeable = requires(T a)
{
    // requires an `order` static member.
    T::order;
    // requires a `serialize` method.
    { a.serialize() };
}; // concept Serializeable

// Requires the type to have a `Timestamp` type internal
template <typename T>
concept MarketData = requires(T a)
{
    // requires type `Timestamp`.
    typename T::Timestamp;
}; // concept MarketData

template <typename T>
concept SerializeableData = requires(T a)
{
    requires MarketData<T>;
    requires Serializeable<T>;
};


class MarketDataCoinbase {
  public:
    // PUBLIC TYPES
    using Timestamp = unsigned long long;
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
    Timestamp   d_sequence;

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
void MarketDataCoinbase::serialize(Archive          & archive,
                                   const unsigned int version)
{
    archive& d_symbol;
    archive& d_price;
    archive& d_sequence;
}

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_TYPES
