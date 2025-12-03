#ifndef INCLUDED_POSITION_MANAGER
#define INCLUDED_POSITION_MANAGER

#include "../common/types.h"

#include <cstdint>
#include <spdlog/spdlog.h>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace crypto_trader {
namespace traders {

struct SymbolPositionData {
    double d_currentHoldings; // Current total amount of product held
    double d_totalCostBasis;  // Sum of (price * amount) for all currently held
                              // positions
    double d_realizedPnl;     // Total realized profit/loss

    SymbolPositionData()
    : d_currentHoldings(0.0)
    , d_totalCostBasis(0.0)
    , d_realizedPnl(0.0)
    {
    }
};

class PositionManager {
  private:
    // PRIVATE TYPES
    struct Position {
        std::string d_symbol;
        int64_t     d_timestamp;
        double      d_price;
        double      d_amount; // Positive for buy, negative for sell
    };

    // PRIVATE DATA
    std::unordered_map<std::string, std::vector<Position>> d_positions;
    std::unordered_map<std::string, SymbolPositionData> d_symbolPositionData;

  public:
    PositionManager();

    // Add a trade to the position manager for a specific symbol.
    // 'amount' is positive for a buy, negative for a sell.
    template <common::TimestampLike Timestamp>
    void addTrade(const std::string_view& symbol,
                  double                  amount,
                  double                  price,
                  const Timestamp&        timestamp);

    // Get the current total holdings for a specific symbol.
    std::optional<double>
    currentHoldings(const std::string_view& symbol) const;

    // Get the average cost basis of the current holdings for a specific
    // symbol. Returns std::nullopt if there are no holdings for the symbol.
    std::optional<double>
    averageCostBasis(const std::string_view& symbol) const;

    // Get the total Profit and Loss for a specific symbol.
    // This profit is either realized or unrealized depending on if the 'price'
    // is given
    std::optional<double> realizedPnl(const std::string_view& symbol) const;
    std::optional<double> unrealizedPnl(const std::string_view& symbol,
                                        double currentPrice) const;

    // Clear all positions and reset PnL for a specific symbol.
    void clear(const std::string_view& symbol);

    // Clear all positions and reset PnL for all symbols.
    void clearAll();
};

template <common::TimestampLike Timestamp>
void PositionManager::addTrade(const std::string_view& symbol,
                               double                  amount,
                               double                  price,
                               const Timestamp&        timestamp)
{
    if (amount == 0) {
        return;
    }

    std::string symbol_s = std::string(symbol);

    SymbolPositionData& symbolData = d_symbolPositionData[symbol_s];

    Position newTrade = {symbol_s, timestamp, price, amount};
    d_positions[symbol_s].push_back(newTrade);

    if (amount > 0) { // Buy
        symbolData.d_totalCostBasis += (price * amount);
        symbolData.d_currentHoldings += amount;
    }
    else {                               // Sell
        double quantityToSell = -amount; // Absolute value of amount
        if (symbolData.d_currentHoldings >= quantityToSell) {
            // Calculate PnL for the sold portion based on average cost basis
            if (symbolData.d_currentHoldings > 0) {
                double avgCost =
                    symbolData.d_totalCostBasis / symbolData.d_currentHoldings;
                symbolData.d_realizedPnl += (price - avgCost) * quantityToSell;
            }
            symbolData.d_currentHoldings -= quantityToSell;
            symbolData.d_totalCostBasis -=
                (symbolData.d_totalCostBasis /
                 (symbolData.d_currentHoldings + quantityToSell)) *
                quantityToSell; // Adjust cost basis proportionately
        }
        else {
            // This case should ideally not happen if a proper strategy is in
            // place but handle it to prevent negative holdings.
            SPDLOG_WARN(
                "Attempted to sell more than held for symbol {}. Selling all "
                "holdings and realizing PnL.",
                symbol);
            if (symbolData.d_currentHoldings > 0) {
                double avgCost =
                    symbolData.d_totalCostBasis / symbolData.d_currentHoldings;
                symbolData.d_realizedPnl +=
                    (price - avgCost) * symbolData.d_currentHoldings;
                symbolData.d_totalCostBasis  = 0.0;
                symbolData.d_currentHoldings = 0.0;
            }
        }
    }

    // If holdings drop to zero or below, reset cost basis
    if (symbolData.d_currentHoldings <= 0) {
        symbolData.d_currentHoldings = 0.0;
        symbolData.d_totalCostBasis  = 0.0;
    }
}

} // namespace traders
} // namespace crypto_trader

#endif // INCLUDED_POSITION_MANAGER
