#ifndef INCLUDED_TRADER
#define INCLUDED_TRADER

#include "../common/types.h"

#include <string_view>

namespace crypto_trader {
namespace protocols {

template <common::MarketData T>
class Loadable {
public:
    // PUBLIC METHODS
    virtual void loadData(std::vector<T> data) = 0;
}; // Loadable

class Trader {

  public:
    // CREATORS
    Trader();
    virtual ~Trader() = 0;

    // PUBLIC MANIPULATORS
    virtual void  listen(const std::string_view &buffer) = 0;
    virtual void  start()                                = 0;
    virtual void  stop()                                 = 0;
    virtual float profit() const                         = 0;

}; // Trader

} // namespace protocols
} // namespace crypto_trader
#endif // INCLUDED_TRADER
