#include "Accounting.h"
#include <gtest/gtest.h>
TEST(AccountingTest, FIFOSellLogic)
{
    Accounting accounting;

    // Buy 10 @ $100
    Event buy1 = {.d_symbol  = "BTC-USD",
                  .d_qty     = 10.0,
                  .d_price   = 100.0,
                  .d_type    = EventType::ORDER_FILLED,
                  .d_payload = {}};
    accounting.apply_event(buy1);

    // Buy 5 @ $120
    Event buy2 = {.d_symbol  = "BTC-USD",
                  .d_qty     = 5.0,
                  .d_price   = 120.0,
                  .d_type    = EventType::ORDER_FILLED,
                  .d_payload = {}};
    accounting.apply_event(buy2);

    auto snapshot = accounting.snapshot();
    EXPECT_NEAR(snapshot.at("BTC-USD").d_total_qty, 15.0, 1e-9);
    EXPECT_NEAR(snapshot.at("BTC-USD").d_average_price,
                106.666,
                1e-3); // (10*100 + 5*120)/15

    // Sell 12 @ $130 (should use FIFO: 10 from first lot, 2 from second)
    Event sell = {.d_symbol  = "BTC-USD",
                  .d_qty     = -12.0,
                  .d_price   = 130.0,
                  .d_type    = EventType::ORDER_FILLED,
                  .d_payload = {}};
    accounting.apply_event(sell);

    snapshot = accounting.snapshot();
    EXPECT_NEAR(snapshot.at("BTC-USD").d_total_qty, 3.0, 1e-9); // 15 - 12
    EXPECT_NEAR(snapshot.at("BTC-USD").d_average_price,
                120.0,
                1e-9); // Only second lot remains

    // Realized PnL: (130-100)*10 + (130-120)*2 = 300 + 20 = 320
    EXPECT_NEAR(snapshot.at("BTC-USD").d_realizedPnl, 320.0, 1e-9);
}

TEST(AccountingTest, ReplayEvents)
{
    Accounting         accounting;
    std::vector<Event> events;

    events.push_back({.d_symbol    = "BTC-USD",
                      .d_qty       = 1.0,
                      .d_price     = 50000.0,
                      .d_type      = EventType::ORDER_FILLED,
                      .d_payload   = {},
                      .d_timestamp = 1});
    events.push_back({.d_symbol    = "BTC-USD",
                      .d_qty       = 0.5,
                      .d_price     = 52000.0,
                      .d_type      = EventType::ORDER_FILLED,
                      .d_payload   = {},
                      .d_timestamp = 2});
    events.push_back({.d_symbol    = "ETH-USD",
                      .d_qty       = 10.0,
                      .d_price     = 4000.0,
                      .d_type      = EventType::ORDER_FILLED,
                      .d_payload   = {},
                      .d_timestamp = 3});
    events.push_back({.d_symbol    = "BTC-USD",
                      .d_qty       = -0.2,
                      .d_price     = 53000.0,
                      .d_type      = EventType::ORDER_FILLED,
                      .d_payload   = {},
                      .d_timestamp = 4});

    accounting.replay_events(events);

    auto snapshot = accounting.snapshot();

    ASSERT_EQ(snapshot.size(), 2);

    auto btc_position = snapshot.at("BTC-USD");
    EXPECT_NEAR(btc_position.d_total_qty, 1.3, 1e-9);
    EXPECT_NEAR(btc_position.d_average_price, 50769.230769230766, 1e-6);

    auto eth_position = snapshot.at("ETH-USD");
    EXPECT_NEAR(eth_position.d_total_qty, 10.0, 1e-9);
    EXPECT_NEAR(eth_position.d_average_price, 4000.0, 1e-9);
}
