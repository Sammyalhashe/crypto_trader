#ifndef INCLUDED_PROTOCOLS_STRATEGY
#define INCLUDED_PROTOCOLS_STRATEGY

#include "../common/types.h"

#include <string_view>

#include <nlohmann/json.hpp>

namespace crypto_trader {
namespace protocols {

class Strategy {

  protected:
    // PROTECTED DATA
    common::Emit d_emit;

  public:
    // CREATORS
    Strategy(const common::Emit& emit);
    virtual ~Strategy() = 0;

    // MANIPULATORS
    virtual void handleNewData(const nlohmann::json& data) = 0;

}; // Strategy
} // namespace protocols
} // namespace crypto_trader

#endif // INCLUDED_PROTOCOLS_STRATEGY
