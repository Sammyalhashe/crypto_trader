#ifndef INCLUDED_COMMON_MATH
#define INCLUDED_COMMON_MATH

#include <cmath>
#include <limits>
#include <type_traits>

namespace crypto_trader {
namespace common {

class Math {
  public:
    static constexpr double EPSILON = 1e-9;

    static bool isEqual(double a, double b, double epsilon = EPSILON)
    {
        return std::abs(a - b) < epsilon;
    }

    static bool isNotEqual(double a, double b, double epsilon = EPSILON)
    {
        return !isEqual(a, b, epsilon);
    }

    static bool isLess(double a, double b, double epsilon = EPSILON)
    {
        return a < b - epsilon;
    }

    static bool isGreater(double a, double b, double epsilon = EPSILON)
    {
        return a > b + epsilon;
    }

    static bool isLessOrEqual(double a, double b, double epsilon = EPSILON)
    {
        return a < b + epsilon;
    }

    static bool isGreaterOrEqual(double a, double b, double epsilon = EPSILON)
    {
        return a > b - epsilon;
    }

    static bool isZero(double a, double epsilon = EPSILON)
    {
        return std::abs(a) < epsilon;
    }
};

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_COMMON_MATH
