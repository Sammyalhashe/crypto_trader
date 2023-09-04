#include "hodl.h"

#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

#include <string_view>

using namespace crypto_trader;
using namespace strategies;

using SV = std::string_view;
using Json = nlohmann::json;

namespace {

    void buildNewSocketMessage(Json      *json,
                               const SV&  type,
                               const SV&  price,
                               const SV&  time)
    {
        (*json)["type"] = type;
        (*json)["price"] = price;
        (*json)["time"] = time;
    }

} // closing unnamed namespace

// TODO: Docs...
TEST(HodlStrategyTest, XPercentRiseTest) {
    HodlStrategyConfig config;
    config.setPercentUp(5)
          .setInitStrategy(HodlStrategyConfig::e_BUY_IMMEDIATELY);
    HodlStrategy hodl(config);
    
    Json data;
    buildNewSocketMessage(&data,
                          "ticker",
                          "1600",
                          "2023-09-04T18:38:48.279032Z");

    hodl.handleNewData(data.dump());

    EXPECT_EQ(1, hodl.trades().size());

    buildNewSocketMessage(&data,
                          "ticker",
                          "1680",
                          "2023-09-04T18:38:49.279032Z");

    hodl.handleNewData(data.dump());

    EXPECT_EQ(0, hodl.trades().size());
}

TEST(HodlStrategyTest, YPercentFallTest) {
    HodlStrategyConfig config;
    config.setPercentDown(5)
          .setInitStrategy(HodlStrategyConfig::e_BUY_IMMEDIATELY);
    HodlStrategy hodl(config);
    
    Json data;
    buildNewSocketMessage(&data,
                          "ticker",
                          "1600",
                          "2023-09-04T18:38:48.279032Z");

    hodl.handleNewData(data.dump());

    EXPECT_EQ(1, hodl.trades().size());

    buildNewSocketMessage(&data,
                          "ticker",
                          "1520",
                          "2023-09-04T18:38:49.279032Z");

    hodl.handleNewData(data.dump());

    EXPECT_EQ(2, hodl.trades().size());

    buildNewSocketMessage(&data,
                          "ticker",
                          "1444",
                          "2023-09-04T18:38:50.279032Z");

    hodl.handleNewData(data.dump());

    EXPECT_EQ(2, hodl.trades().size());
}

TEST(HodlStrategyTest, BASIS_PRICE_INIT) {
    HodlStrategyConfig config;
    config.setPercentDown(5)
          .setPercentUp(5)
          .setInitStrategy(HodlStrategyConfig::e_SET_BASIS_PRICE);
    HodlStrategy hodl(config);
    
    Json data;
    buildNewSocketMessage(&data,
                          "ticker",
                          "1600",
                          "2023-09-04T18:38:48.279032Z");

    hodl.handleNewData(data.dump());

    EXPECT_EQ(1600, hodl.basisMarketPrice().value());
    EXPECT_EQ(0, hodl.trades().size());

    buildNewSocketMessage(&data,
                          "ticker",
                          "1520",
                          "2023-09-04T18:38:49.279032Z");

    hodl.handleNewData(data.dump());
    EXPECT_EQ(1, hodl.trades().size());
    EXPECT_EQ(false, hodl.basisMarketPrice().has_value());
    
    buildNewSocketMessage(&data,
                          "ticker",
                          "1444",
                          "2023-09-04T18:38:50.279032Z");

    hodl.handleNewData(data.dump());
    EXPECT_EQ(2, hodl.trades().size());
    EXPECT_EQ(false, hodl.basisMarketPrice().has_value());

    buildNewSocketMessage(&data,
                          "ticker",
                          "1378",
                          "2023-09-04T18:38:51.279032Z");

    hodl.handleNewData(data.dump());
    EXPECT_EQ(2, hodl.trades().size());
    EXPECT_EQ(false, hodl.basisMarketPrice().has_value());

    buildNewSocketMessage(&data,
                          "ticker",
                          "1520",
                          "2023-09-04T18:38:52.279032Z");

    hodl.handleNewData(data.dump());
    EXPECT_EQ(1, hodl.trades().size());
    EXPECT_EQ(false, hodl.basisMarketPrice().has_value());

    buildNewSocketMessage(&data,
                          "ticker",
                          "1600",
                          "2023-09-04T18:38:52.279032Z");

    hodl.handleNewData(data.dump());
    EXPECT_EQ(0, hodl.trades().size());
    EXPECT_EQ(true, hodl.basisMarketPrice().has_value());
    EXPECT_EQ(1600, hodl.basisMarketPrice().value());
}
