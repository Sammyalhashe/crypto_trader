#include "position_manager.h"

#include <numeric>
#include <optional>
#include <spdlog/spdlog.h>
#include <string_view>

namespace crypto_trader {
namespace traders {

PositionManager::PositionManager()
{
    // No-op for now, as data is initialized on first symbol access
}

std::optional<double>
PositionManager::currentHoldings(const std::string_view& symbol) const
{
    auto it = d_symbolPositionData.find(std::string(symbol));
    if (it != d_symbolPositionData.end()) {
        return it->second.d_currentHoldings;
    }
    return std::nullopt;
}

std::optional<double>
PositionManager::averageCostBasis(const std::string_view& symbol) const
{
    auto it = d_symbolPositionData.find(std::string(symbol));
    if (it != d_symbolPositionData.end() && it->second.d_currentHoldings > 0) {
        return it->second.d_totalCostBasis / it->second.d_currentHoldings;
    }
    return std::nullopt;
}

std::optional<double>
PositionManager::realizedPnl(const std::string_view& symbol) const
{

    auto it = d_symbolPositionData.find(std::string(symbol));
    if (it != d_symbolPositionData.end()) {
        return it->second.d_realizedPnl;
    }
    return std::nullopt;
}
std::optional<double>
PositionManager::unrealizedPnl(const std::string_view& symbol,
                               double                  currentPrice) const
{
    auto it = d_symbolPositionData.find(std::string(symbol));

    if (it != d_symbolPositionData.end()) {
        if (it->second.d_currentHoldings > 0) {
            double avgCost =
                it->second.d_totalCostBasis / it->second.d_currentHoldings;
            return (currentPrice - avgCost) * it->second.d_currentHoldings;
        }
    }
    return std::nullopt;
}

void PositionManager::clear(const std::string_view& symbol)
{
    auto s = std::string(symbol);
    d_positions.erase(s);
    d_symbolPositionData.erase(s);
}

void PositionManager::clearAll()
{
    d_positions.clear();
    d_symbolPositionData.clear();
}

} // namespace traders
} // namespace crypto_trader
