/**
 * @module risk_manager
 * @brief Defines the RiskManager interface for risk approval in crypto trading protocols.
 *
 * This module provides an abstract base class `RiskManager` within the `crypto_trader::protocols` namespace.
 * The `RiskManager` interface is designed to be subclassed by concrete risk management implementations.
 * It exposes a single pure virtual method, `approve`, which determines whether a given trading event
 * is permitted based on the current state of the `PositionManager`.
 *
 * Usage:
 * - Inherit from `RiskManager` and implement the `approve` method to define custom risk logic.
 * - The `approve` method receives an event and a pointer to the position manager, returning `true`
 *   if the event is approved, or `false` otherwise.
 *
 * Example:
 * @code
 * class MyRiskManager : public RiskManager {
 * public:
 *     bool approve(const common::Event& event, PositionManager* const positionManager) override {
 *         // Custom risk logic
 *         return true;
 *     }
 * };
 * @endcode
 */
```
module;

#include "../common/Event.h"
#include "position_manager.h"

export module risk_manager;

namespace crypto_trader {
namespace protocols {
export class RiskManager {
public:
    // CREATORS
    RiskManager() = default;
    virtual ~RiskManager() = 0;

    // PUBLIC ACCESSORS
    virtual bool approve(const common::Event& event, PositionManager * const positionManager) = 0;


}; // class RiskManager

// CREATORS
inline RiskManager::~RiskManager() {}

} // closing namespace protocols
} // closing namespace crypto_trader
