#include "hodl.h"

#include <nlohmann/json.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <functional>
#include <string_view>

#include "../traders/event_position_manager.h"

using namespace crypto_trader;
using namespace strategies;
using namespace traders;
using namespace testing;

using SV   = std::string_view;
using Json = nlohmann::json;

namespace {

void buildNewSocketMessage(Json     *json,
                           const SV& type,
                           const SV& price,
                           const SV& time,
                           const SV& product)
{
    (*json)["type"]       = type;
    (*json)["price"]      = price;
    (*json)["time"]       = time;
    (*json)["product_id"] = product;
}

void dummyHandleAction(const common::Action& action) {}


class MockEventPositionManager : public EventPositionManager {
public:
    MOCK_METHOD(std::optional<double>, currentHoldings, (const std::string_view& symbol), (const override));
    MOCK_METHOD(void, submit_event, (const Event& e), (override));
};

} // namespace

// TODO: Docs...
TEST(HodlStrategyTest, XPercentRiseTest)
{
    HodlStrategyConfig config;
    config.setPercentUp(5)
        .setPercentDown(5)
        .setInitStrategy(HodlStrategyConfig::e_BUY_IMMEDIATELY)
        .setEmit(std::bind(&dummyHandleAction, std::placeholders::_1));
    
    MockEventPositionManager mock_pm;
    ON_CALL(mock_pm, currentHoldings(_)).WillByDefault(Return(1.0));
    EXPECT_CALL(mock_pm, submit_event(_)).Times(AtLeast(0));

    HodlStrategy hodl(config, mock_pm);

    std::string product("ETH-USD");
    Json        data;
    buildNewSocketMessage(
        &data, "ticker", "1600", "2023-09-04T18:38:48.279032Z", product);

    hodl.handleNewData(data);

    // EXPECT_EQ(1, hodl.tradesForProduct().at(product).size());

    buildNewSocketMessage(
        &data, "ticker", "1680", "2023-09-04T18:38:49.279032Z", product);

    hodl.handleNewData(data);

    // EXPECT_EQ(0, hodl.tradesForProduct().at(product).size());
}

TEST(HodlStrategyTest, YPercentFallTest)
{
    HodlStrategyConfig config;
    config.setPercentDown(5)
        .setPercentUp(5)
        .setInitStrategy(HodlStrategyConfig::e_BUY_IMMEDIATELY)
        .setEmit(std::bind(&dummyHandleAction, std::placeholders::_1));
    
    MockEventPositionManager mock_pm;
    ON_CALL(mock_pm, currentHoldings(_)).WillByDefault(Return(1.0));
    EXPECT_CALL(mock_pm, submit_event(_)).Times(AtLeast(0));

    HodlStrategy hodl(config, mock_pm);

    std::string product("ETH-USD");

    Json data;
    buildNewSocketMessage(
        &data, "ticker", "1600", "2023-09-04T18:38:48.279032Z", product);

    hodl.handleNewData(data);

    // EXPECT_EQ(1, hodl.tradesForProduct().at(product).size());

    buildNewSocketMessage(
        &data, "ticker", "1519", "2023-09-04T18:38:49.279032Z", product);

    hodl.handleNewData(data);

    // EXPECT_EQ(2, hodl.tradesForProduct().at(product).size());

    buildNewSocketMessage(
        &data, "ticker", "1443", "2023-09-04T18:38:50.279032Z", "ETH-USD");

    hodl.handleNewData(data);

    // EXPECT_EQ(2, hodl.tradesForProduct().at(product).size());
}

TEST(HodlStrategyTest, BASIS_PRICE_INIT)
{
    HodlStrategyConfig config;
    config.setPercentDown(5)
        .setPercentUp(5)
        .setInitStrategy(HodlStrategyConfig::e_SET_BASIS_PRICE)
        .setEmit(std::bind(&dummyHandleAction, std::placeholders::_1));
    
    MockEventPositionManager mock_pm;
    ON_CALL(mock_pm, currentHoldings(_)).WillByDefault(Return(0.0));
    EXPECT_CALL(mock_pm, submit_event(_)).Times(AtLeast(0));

    HodlStrategy hodl(config, mock_pm);

    std::string product("ETH-USD");
    Json        data;
    buildNewSocketMessage(
        &data, "ticker", "1600", "2023-09-04T18:38:48.279032Z", product);

    hodl.handleNewData(data);

    // EXPECT_EQ(1600, hodl.basisMarketPrices().at(product));
    // EXPECT_EQ(0, hodl.tradesForProduct().at(product).size());

    buildNewSocketMessage(
        &data, "ticker", "1519", "2023-09-04T18:38:49.279032Z", product);

    hodl.handleNewData(data);
    // EXPECT_EQ(1, hodl.tradesForProduct().at(product).size());
    // EXPECT_EQ(false,
    //           hodl.basisMarketPrices().find(product) !=
    //               hodl.basisMarketPrices().end());

    buildNewSocketMessage(
        &data, "ticker", "1443", "2023-09-04T18:38:50.279032Z", product);

    hodl.handleNewData(data);
    // EXPECT_EQ(2, hodl.tradesForProduct().at(product).size());
    // EXPECT_EQ(false,
    //           hodl.basisMarketPrices().find(product) !=
    //               hodl.basisMarketPrices().end());

    buildNewSocketMessage(
        &data, "ticker", "1378", "2023-09-04T18:38:51.279032Z", product);

    hodl.handleNewData(data);
    // EXPECT_EQ(2, hodl.tradesForProduct().at(product).size());
    // EXPECT_EQ(false,
    //           hodl.basisMarketPrices().find(product) !=
    //               hodl.basisMarketPrices().end());

    buildNewSocketMessage(
        &data, "ticker", "1520", "2023-09-04T18:38:52.279032Z", product);

    hodl.handleNewData(data);
    // EXPECT_EQ(1, hodl.tradesForProduct().at(product).size());
    // EXPECT_EQ(false,
    //           hodl.basisMarketPrices().find(product) !=
    //               hodl.basisMarketPrices().end());

    buildNewSocketMessage(
        &data, "ticker", "1600", "2023-09-04T18:38:52.279032Z", product);

    hodl.handleNewData(data);
    // EXPECT_EQ(0, hodl.tradesForProduct().at(product).size());
    // EXPECT_EQ(true,
    //           hodl.basisMarketPrices().find(product) !=
    //               hodl.basisMarketPrices().end());
    // EXPECT_EQ(1600.0, hodl.basisMarketPrices().find(product)->second);
}
