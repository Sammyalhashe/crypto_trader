#ifndef INCLUDED_COMMON_MATH
#define INCLUDED_COMMON_MATH

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

namespace crypto_trader {
namespace common {

class Math {
  public:
    static constexpr double DEFAULT_REL_EPSILON = 1e-9;
    static constexpr double DEFAULT_ABS_EPSILON = 1e-12;

    static bool isEqual(double a,
                        double b,
                        double relEpsilon = DEFAULT_REL_EPSILON,
                        double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        double diff = std::abs(a - b);
        if (diff < absEpsilon)
            return true;
        return diff <= (std::max(std::abs(a), std::abs(b)) * relEpsilon);
    }

    static bool isNotEqual(double a,
                           double b,
                           double relEpsilon = DEFAULT_REL_EPSILON,
                           double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return !isEqual(a, b, relEpsilon, absEpsilon);
    }

    static bool isLess(double a,
                       double b,
                       double relEpsilon = DEFAULT_REL_EPSILON,
                       double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return a < b && !isEqual(a, b, relEpsilon, absEpsilon);
    }

    static bool isGreater(double a,
                          double b,
                          double relEpsilon = DEFAULT_REL_EPSILON,
                          double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return a > b && !isEqual(a, b, relEpsilon, absEpsilon);
    }

    static bool isLessOrEqual(double a,
                              double b,
                              double relEpsilon = DEFAULT_REL_EPSILON,
                              double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return a < b || isEqual(a, b, relEpsilon, absEpsilon);
    }

    static bool isGreaterOrEqual(double a,
                                 double b,
                                 double relEpsilon = DEFAULT_REL_EPSILON,
                                 double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return a > b || isEqual(a, b, relEpsilon, absEpsilon);
    }

    static bool isZero(double a, double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return std::abs(a) < absEpsilon;
    }
};

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_COMMON_MATH