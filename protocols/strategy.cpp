#include "strategy.h"

namespace crypto_trader {
namespace protocols {
// class Strategy
// CREATORS
Strategy::Strategy(const common::Emit& emit)
: d_emit(emit)
{
}

Strategy::~Strategy()
{
}

} // protocols
} // crypto_trader
