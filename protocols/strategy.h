#ifndef INCLUDED_PROTOCOLS_STRATEGY
#define INCLUDED_PROTOCOLS_STRATEGY

#include "../common/types.h"

#include <string_view>

#include <nlohmann/json.hpp>

namespace crypto_trader {
namespace protocols {

/**
 * @brief Abstract base class defining the interface for a trading strategy.
 *
 * Strategies analyze market data and emit trading actions.
 */
class Strategy {

  protected:
    common::Emit d_emit; //!< Function to emit trading actions.

  public:
    /**
     * @brief Constructs a Strategy object.
     * @param emit A function callback used by the strategy to emit actions.
     */
    Strategy(const common::Emit& emit);

    /**
     * @brief Destructor for the Strategy interface.
     */
    virtual ~Strategy() = 0;

    /**
     * @brief Handles new market data received by the strategy.
     * @param data The new market data, typically in JSON format.
     */
    virtual void handleNewData(const nlohmann::json& data) = 0;

}; // Strategy
} // namespace protocols
} // namespace crypto_trader

#endif // INCLUDED_PROTOCOLS_STRATEGY