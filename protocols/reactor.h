#ifndef INCLUDED_PROTOCOLS_REACTOR
#define INCLUDED_PROTOCOLS_REACTOR

namespace crypto_trader {
namespace protocols {

class Reactor {

    // Callback function that is called by the thing an instance of this class
    // is "reacting" to.
    virtual void react() const = 0;

}; // class Reactor
    
} // closing namespace protocols
} // closing namespace crypto_trader

#endif // INCLUDED_PROTOCOLS_REACTOR

