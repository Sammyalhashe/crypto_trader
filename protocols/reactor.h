#ifndef INCLUDED_PROTOCOLS_REACTOR
#define INCLUDED_PROTOCOLS_REACTOR

#include "../common/types.h"

namespace crypto_trader {
namespace protocols {

template <common::MarketData MarketDataType>
class Reactor {
  public:
    // CREATORS
    Reactor();
    virtual ~Reactor() = 0;

    // Callback function that is called by the thing an instance of this class
    // is "reacting" to.
    virtual void react(MarketDataType data) const = 0;

}; // class Reactor

template <common::MarketData MarketDataType>
class Changer {
  public:
    // CREATORS
    Changer();
    virtual ~Changer() = 0;

    // Function to notify all `reactors` of data changes.
    virtual void notifyAllReactors(MarketDataType data) = 0;

    // Register reactor to be notified of data changes
    virtual bool registerReactor(Reactor<MarketDataType> *reactor) = 0;

    // Unregister reactor to be notified of data changes
    virtual bool unregisterReactor(Reactor<MarketDataType> *reactor) = 0;

}; // class Changer

// class Reactor

// CREATORS
template <common::MarketData MarketDataType>
Reactor<MarketDataType>::Reactor()
{
}

template <common::MarketData MarketDataType>
Reactor<MarketDataType>::~Reactor()
{
}

// class Changer

// CREATORS
template <common::MarketData MarketDataType>
Changer<MarketDataType>::Changer()
{
}

template <common::MarketData MarketDataType>
Changer<MarketDataType>::~Changer()
{
}

} // namespace protocols
} // namespace crypto_trader

#endif // INCLUDED_PROTOCOLS_REACTOR
