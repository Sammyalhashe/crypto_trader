#ifndef INCLUDED_PROTOCOLS_STRATEGY
#define INCLUDED_PROTOCOLS_STRATEGY

#include "../common/types.h"

#include <string_view>

namespace crypto_trader {
namespace protocols {

class Strategy {

protected:
    // PROTECTED DATA
    common::Types::Emit d_emit;

public:
    // CREATORS
    Strategy(const common::Types::Emit&);
    virtual ~Strategy() = 0;

    // MANIPULATORS
    virtual void handleNewData(const std::string_view& buffer) = 0;

};// Strategy
} // protocols
} // crypto_trader

#endif // INCLUDED_PROTOCOLS_STRATEGY
