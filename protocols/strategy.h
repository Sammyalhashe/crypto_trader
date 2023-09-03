#ifndef INCLUDED_PROTOCOLS_STRATEGY
#define INCLUDED_PROTOCOLS_STRATEGY

#include <string_view>

namespace crypto_trader {
namespace protocols {

class Strategy {

public:
    // CREATORS
    Strategy();
    virtual ~Strategy() = 0;

    // MANIPULATORS
    virtual void handleNewData(const std::string_view& buffer) = 0;

};// Strategy
} // protocols
} // crypto_trader

#endif // INCLUDED_PROTOCOLS_STRATEGY
