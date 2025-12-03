#pragma once

#include "../common/types.h"

#include <optional>
#include <string_view>

namespace crypto_trader {
namespace protocols {

template <common::MarketData T>
class Executor {
  public:
    virtual ~Executor() = default;

    // Execute a buy order for a given product and quantity
    virtual common::TradeResult buy(const std::string_view& product,
                                    double                  quantity) = 0;

    // Execute a sell order for a given product and quantity
    virtual common::TradeResult sell(const std::string_view& product,
                                     double                  quantity) = 0;

    // Get the current balance of a currency (e.g., USD, BTC)
    virtual double getBalance(const std::string_view& currency) const = 0;

    // Get the current position for a given product (e.g., how much BTC-USD is
    // held)
    virtual std::optional<double>
    getPosition(const std::string_view& product) const = 0;

    virtual void processTickerData(const std::string_view& product,
                                   double                  price,
                                   const T::Timestamp&     timestamp) = 0;
};

} // namespace protocols
} // namespace crypto_trader
