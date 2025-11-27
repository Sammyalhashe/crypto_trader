#include <gtest/gtest.h>
#include "Accounting.h"

TEST(AccountingTest, ReplayEvents) {
    Accounting accounting;
    std::vector<Event> events;

    events.push_back({.d_symbol = "BTC-USD", .d_qty = 1.0, .d_price = 50000.0, .d_type = EventType::ORDER_FILLED, .d_payload = {}});
    events.push_back({.d_symbol = "BTC-USD", .d_qty = 0.5, .d_price = 52000.0, .d_type = EventType::ORDER_FILLED, .d_payload = {}});
    events.push_back({.d_symbol = "ETH-USD", .d_qty = 10.0, .d_price = 4000.0, .d_type = EventType::ORDER_FILLED, .d_payload = {}});
    events.push_back({.d_symbol = "BTC-USD", .d_qty = -0.2, .d_price = 53000.0, .d_type = EventType::ORDER_FILLED, .d_payload = {}});

    accounting.replay_events(events);

    auto snapshot = accounting.snapshot();

    ASSERT_EQ(snapshot.size(), 2);

    auto btc_position = snapshot.at("BTC-USD");
    EXPECT_NEAR(btc_position.d_total_qty, 1.3, 1e-9);
    EXPECT_NEAR(btc_position.d_average_price, 50666.666666666664, 1e-6);

    auto eth_position = snapshot.at("ETH-USD");
    EXPECT_NEAR(eth_position.d_total_qty, 10.0, 1e-9);
    EXPECT_NEAR(eth_position.d_average_price, 4000.0, 1e-9);
}
