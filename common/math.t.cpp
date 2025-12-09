#include "math.h"
#include <gtest/gtest.h>

using namespace crypto_trader::common;

TEST(MathTest, IsEqual)
{
    // Basic equality
    EXPECT_TRUE(Math::isEqual(1.0, 1.0));
    EXPECT_TRUE(Math::isEqual(0.0, 0.0));
    EXPECT_TRUE(Math::isEqual(-1.0, -1.0));

    // Within absolute epsilon (near zero)
    EXPECT_TRUE(Math::isEqual(1e-13, 0.0)); // Default absEpsilon is 1e-12
    EXPECT_TRUE(Math::isEqual(0.0, 1e-13));

    // Within relative epsilon
    double a = 1000.0;
    double b = 1000.0 + 1e-7; // Difference is 1e-7, relative error 1e-10
    // Default relEpsilon is 1e-9
    EXPECT_TRUE(Math::isEqual(a, b));

    // Outside limits
    EXPECT_FALSE(
        Math::isEqual(1.0, 1.0 + 1e-8)); // Difference > relEpsilon for 1.0
}

TEST(MathTest, IsNotEqual)
{
    EXPECT_TRUE(Math::isNotEqual(1.0, 2.0));
    EXPECT_FALSE(Math::isNotEqual(1.0, 1.0));
    EXPECT_TRUE(Math::isNotEqual(1.0, 1.0 + 1e-8));
}

TEST(MathTest, IsLess)
{
    EXPECT_TRUE(Math::isLess(1.0, 2.0));
    EXPECT_FALSE(Math::isLess(2.0, 1.0));
    EXPECT_FALSE(Math::isLess(1.0, 1.0)); // Strict inequality

    // Tiny difference considered equal, so not less
    EXPECT_FALSE(Math::isLess(1.0, 1.0 + 1e-10));
}

TEST(MathTest, IsGreater)
{
    EXPECT_TRUE(Math::isGreater(2.0, 1.0));
    EXPECT_FALSE(Math::isGreater(1.0, 2.0));
    EXPECT_FALSE(Math::isGreater(1.0, 1.0));

    // Tiny difference considered equal, so not greater
    EXPECT_FALSE(Math::isGreater(1.0 + 1e-10, 1.0));
}

TEST(MathTest, IsLessOrEqual)
{
    EXPECT_TRUE(Math::isLessOrEqual(1.0, 2.0));
    EXPECT_TRUE(Math::isLessOrEqual(1.0, 1.0));
    EXPECT_FALSE(Math::isLessOrEqual(2.0, 1.0));

    // Tiny difference considered equal, so true
    EXPECT_TRUE(Math::isLessOrEqual(1.0, 1.0 + 1e-10));
}

TEST(MathTest, IsGreaterOrEqual)
{
    EXPECT_TRUE(Math::isGreaterOrEqual(2.0, 1.0));
    EXPECT_TRUE(Math::isGreaterOrEqual(1.0, 1.0));
    EXPECT_FALSE(Math::isGreaterOrEqual(1.0, 2.0));

    // Tiny difference considered equal, so true
    EXPECT_TRUE(Math::isGreaterOrEqual(1.0 + 1e-10, 1.0));
}

TEST(MathTest, IsZero)
{
    EXPECT_TRUE(Math::isZero(0.0));
    EXPECT_TRUE(Math::isZero(1e-13));
    EXPECT_FALSE(Math::isZero(1e-11));
}
