#ifndef INCLUDED_TRADER
#define INCLUDED_TRADER

#include <string_view>

namespace crypto_trader {
namespace protocols {

class Trader {

public:
    // CREATORS
    Trader();
    virtual ~Trader() = 0;

    // PUBLIC MANIPULATORS
    virtual void listen(const std::string_view& buffer) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

}; // Trader

} // protocols
} // crypto_trader
#endif // INCLUDED_TRADER
