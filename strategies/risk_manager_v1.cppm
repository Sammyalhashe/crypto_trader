/**
 * @module risk_manager_v1
 * @brief Provides a concrete implementation of the RiskManager interface for
 * crypto trading strategies.
 *
 * This module defines the `RiskManagerV1` class within the
 * `crypto_trader::strategies` namespace. `RiskManagerV1` inherits from
 * `protocols::RiskManager` and implements the required `approve` method. In
 * its current form, `approve` always returns `true`, allowing all events to
 * pass risk checks.
 *
 * Usage:
 * - Instantiate `RiskManagerV1` and use it wherever a `RiskManager` is
 * required.
 * - Override or modify the `approve` method to implement custom risk logic as
 * needed.
 *
 * Example:
 * @code
 * crypto_trader::strategies::RiskManagerV1 riskManager;
 * bool isApproved = riskManager.approve(event, positionManager);
 * @endcode
 */
module;

#include "../common/Event.h"
#include "../protocols/position_manager.h"

export module risk_manager_v1;

import risk_manager;

namespace crypto_trader {
namespace strategies {

export class RiskManagerV1 : public protocols::RiskManager {
  public:
    // CREATORS
    RiskManagerV1();
    ~RiskManagerV1() override;

    // PUBLIC ACCESSORS
    // RiskManager implementation
    bool approve(const common::Event&              event,
                 protocols::PositionManager *const positionManager) override;
}; // class RiskManager

// class RiskManagerV1

// CREATORS
RiskManagerV1::RiskManagerV1() {}
RiskManagerV1::~RiskManagerV1() {}

// ACCESSORS
// RiskManager implementation

bool RiskManagerV1::approve(const common::Event&              event,
                            protocols::PositionManager *const positionManager)
{
    return true;
}

} // namespace strategies
} // namespace crypto_trader
