#ifndef INCLUDED_CRYPTO_TRADER_PROTOCOLS_OBSERVER_H
#define INCLUDED_CRYPTO_TRADER_PROTOCOLS_OBSERVER_H

#include "../common/types.h"
#include <string>

namespace crypto_trader {
namespace protocols {

class Observer {
public:
    virtual ~Observer() = default;
    virtual void on_trade(const common::Trade& trade) = 0;
    virtual void on_position_update(const std::string& symbol, double new_position) = 0;
};

} // namespace protocols
} // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_PROTOCOLS_OBSERVER_H
