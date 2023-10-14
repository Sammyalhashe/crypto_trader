#include "market_data_db.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <string>

using namespace crypto_trader;

TEST(MarketDataTest, add_and_get_data)
{
    databases::MarketDataDB<common::MarketDataCoinbase> database;

    common::MarketDataCoinbase data_1;
    data_1.d_symbol   = "BTC";
    data_1.d_price    = 1000.00;
    data_1.d_sequence = 5;

    common::MarketDataCoinbase data_2;
    data_2.d_symbol   = "BTC";
    data_2.d_price    = 1002.00;
    data_2.d_sequence = 10;

    common::MarketDataCoinbase data_3;
    data_3.d_symbol   = "ETH";
    data_3.d_price    = 1005.00;
    data_3.d_sequence = 7;

    database.add_data(data_1.d_symbol, data_1);
    database.add_data(data_2.d_symbol, data_2);
    database.add_data(data_3.d_symbol, data_3);

    // inclusive on both ends
    auto vec(database.get_data(data_1.d_symbol, 5, 5));

    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].d_symbol, "BTC");
    EXPECT_FLOAT_EQ(vec[0].d_price, 1000.0);
    EXPECT_EQ(vec[0].d_sequence, 5);

    // gets multiple
    auto vec_2(database.get_data(data_1.d_symbol, 5, 10));

    EXPECT_EQ(vec_2.size(), 2);
    EXPECT_EQ(vec_2[0].d_symbol, "BTC");
    EXPECT_FLOAT_EQ(vec_2[0].d_price, 1000.0);
    EXPECT_EQ(vec_2[0].d_sequence, 5);

    EXPECT_EQ(vec_2[1].d_symbol, "BTC");
    EXPECT_FLOAT_EQ(vec_2[1].d_price, 1002.0);
    EXPECT_EQ(vec_2[1].d_sequence, 10);

    // excludes other d_symbols
    auto vec_3(database.get_data(data_3.d_symbol, 5, 10));

    EXPECT_EQ(vec_3.size(), 1);
    EXPECT_EQ(vec_3[0].d_symbol, "ETH");
    EXPECT_FLOAT_EQ(vec_3[0].d_price, 1005.0);
    EXPECT_EQ(vec_3[0].d_sequence, 7);
}

TEST(MarketDataTest, save_and_load_data)
{
    databases::MarketDataDB<common::MarketDataCoinbase> database;
    common::MarketDataCoinbase                          data_1;
    data_1.d_symbol   = "BTC";
    data_1.d_price    = 1000.00;
    data_1.d_sequence = 5;

    common::MarketDataCoinbase data_2;
    data_2.d_symbol   = "BTC";
    data_2.d_price    = 1002.00;
    data_2.d_sequence = 10;

    common::MarketDataCoinbase data_3;
    data_3.d_symbol   = "ETH";
    data_3.d_price    = 1005.00;
    data_3.d_sequence = 7;

    database.add_data(data_1.d_symbol, data_1);
    database.add_data(data_2.d_symbol, data_2);
    database.add_data(data_3.d_symbol, data_3);

    std::string file_name = "test.db";

    spdlog::info("saving");
    database.save(file_name);

    databases::MarketDataDB<common::MarketDataCoinbase> loaded_database;

    spdlog::info("loading");
    loaded_database.load(file_name);

    auto vec(loaded_database.get_data(data_1.d_symbol, 5, 5));

    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].d_symbol, "BTC");
    EXPECT_FLOAT_EQ(vec[0].d_price, 1000.0);
    EXPECT_EQ(vec[0].d_sequence, 5);
}
