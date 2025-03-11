#ifndef INCLUDED_TYPES
#define INCLUDED_TYPES

#include "serialization.h"

#include <functional>
#include <string>

namespace crypto_trader {

namespace common {

struct Action {
    enum class ActionType { e_BUY = 0, e_SELL = 1 }; // enum class ActionType
    enum class OrderType { e_MARKET, e_LIMIT };      // enum OrderType

    // DATA
    // Type of the emitted action.
    ActionType d_actionType;
    OrderType  d_orderType;

    friend std::ostream& operator<<(std::ostream& out, const Action& action);
}; // struct Action

inline
std::ostream& operator<<(std::ostream& out, const Action& action)
{
    out << std::string("ActionType: ");

    switch (action.d_actionType) {
    case Action::ActionType::e_BUY:
        out << std::string("BUY");
        break;
    case Action::ActionType::e_SELL:
        out << std::string("SELL");
        break;
    default:
        out << std::string("Unknown ActionType");
        break;
    }

    out << std::string("OrderType: ");

    switch (action.d_orderType) {
    case Action::OrderType::e_MARKET:
        out << std::string("MARKET");
        break;
    case Action::OrderType::e_LIMIT:
        out << std::string("LIMIT");
        break;
    default:
        out << std::string("Unknown OrderType");
        break;
    }
    return out;
}


typedef std::function<void(const Action&)> Emit;

template <typename T>
concept Serializeable = requires(T a) {
    // requires an `order` static member.
    T::order;
    // requires a `serialize` method.
    {
        a.serialize()
    };
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
void MarketDataCoinbase::serialize(Archive&           archive,
                                   const unsigned int version)
{
    archive & d_symbol;
    archive & d_price;
    archive & d_sequence;
}

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_TYPES
