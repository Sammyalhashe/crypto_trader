#include "simple.h"

#include <gtest/gtest.h>

TEST(SimpleTest, SimpleTest) {
    EXPECT_EQ(0, crypto_trader::strategies::simpleStrategy());
}
