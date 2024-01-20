#ifndef INCLUDED_PROTOCOLS_REACTOR
#define INCLUDED_PROTOCOLS_REACTOR

namespace crypto_trader {
namespace protocols {

class Reactor {

    // CREATORS
    Reactor();
    virtual ~Reactor() = 0;

    // Callback function that is called by the thing an instance of this class
    // is "reacting" to.
    virtual void react() const = 0;

}; // class Reactor

class Changer {
  public:
    // CREATORS
    Changer();
    virtual ~Changer() = 0;

    // Function to notify all `reactors` of data changes.
    virtual void notifyAllReactors() = 0;

    // Register reactor to be notified of data changes
    virtual bool registerReactor(Reactor *reactor) = 0;

    // Unregister reactor to be notified of data changes
    virtual bool unregisterReactor(Reactor* reactor) = 0;

}; // class Changer

} // namespace protocols
} // namespace crypto_trader
