#ifndef INCLUDED_COMMON_MATH
#define INCLUDED_COMMON_MATH

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

namespace crypto_trader {
namespace common {

/**
 * @brief Provides static utility methods for safe floating-point comparisons.
 *
 * This class implements a hybrid comparison approach (relative and absolute epsilon)
 * to account for the inherent imprecision of floating-point numbers across different scales.
 */
class Math {
  public:
    static constexpr double DEFAULT_REL_EPSILON = 1e-9; //!< Default relative epsilon for comparisons.
    static constexpr double DEFAULT_ABS_EPSILON = 1e-12; //!< Default absolute epsilon for comparisons near zero.

    /**
     * @brief Checks if two double-precision floating-point numbers are considered equal
     *        within a specified relative and/or absolute tolerance.
     *
     * @param a The first double value.
     * @param b The second double value.
     * @param relEpsilon The relative epsilon value. Defaults to DEFAULT_REL_EPSILON.
     * @param absEpsilon The absolute epsilon value. Defaults to DEFAULT_ABS_EPSILON.
     * @return True if the numbers are considered equal, false otherwise.
     */
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

    /**
     * @brief Checks if two double-precision floating-point numbers are considered not equal.
     *
     * @param a The first double value.
     * @param b The second double value.
     * @param relEpsilon The relative epsilon value. Defaults to DEFAULT_REL_EPSILON.
     * @param absEpsilon The absolute epsilon value. Defaults to DEFAULT_ABS_EPSILON.
     * @return True if the numbers are considered not equal, false otherwise.
     */
    static bool isNotEqual(double a,
                           double b,
                           double relEpsilon = DEFAULT_REL_EPSILON,
                           double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return !isEqual(a, b, relEpsilon, absEpsilon);
    }

    /**
     * @brief Checks if the first double value is strictly less than the second,
     *        considering floating-point tolerance.
     *
     * @param a The first double value.
     * @param b The second double value.
     * @param relEpsilon The relative epsilon value. Defaults to DEFAULT_REL_EPSILON.
     * @param absEpsilon The absolute epsilon value. Defaults to DEFAULT_ABS_EPSILON.
     * @return True if 'a' is strictly less than 'b' outside the equality zone, false otherwise.
     */
    static bool isLess(double a,
                       double b,
                       double relEpsilon = DEFAULT_REL_EPSILON,
                       double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return a < b && !isEqual(a, b, relEpsilon, absEpsilon);
    }

    /**
     * @brief Checks if the first double value is strictly greater than the second,
     *        considering floating-point tolerance.
     *
     * @param a The first double value.
     * @param b The second double value.
     * @param relEpsilon The relative epsilon value. Defaults to DEFAULT_REL_EPSILON.
     * @param absEpsilon The absolute epsilon value. Defaults to DEFAULT_ABS_EPSILON.
     * @return True if 'a' is strictly greater than 'b' outside the equality zone, false otherwise.
     */
    static bool isGreater(double a,
                          double b,
                          double relEpsilon = DEFAULT_REL_EPSILON,
                          double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return a > b && !isEqual(a, b, relEpsilon, absEpsilon);
    }

    /**
     * @brief Checks if the first double value is less than or equal to the second,
     *        considering floating-point tolerance.
     *
     * @param a The first double value.
     * @param b The second double value.
     * @param relEpsilon The relative epsilon value. Defaults to DEFAULT_REL_EPSILON.
     * @param absEpsilon The absolute epsilon value. Defaults to DEFAULT_ABS_EPSILON.
     * @return True if 'a' is less than or equal to 'b', false otherwise.
     */
    static bool isLessOrEqual(double a,
                              double b,
                              double relEpsilon = DEFAULT_REL_EPSILON,
                              double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return a < b || isEqual(a, b, relEpsilon, absEpsilon);
    }

    /**
     * @brief Checks if the first double value is greater than or equal to the second,
     *        considering floating-point tolerance.
     *
     * @param a The first double value.
     * @param b The second double value.
     * @param relEpsilon The relative epsilon value. Defaults to DEFAULT_REL_EPSILON.
     * @param absEpsilon The absolute epsilon value. Defaults to DEFAULT_ABS_EPSILON.
     * @return True if 'a' is greater than or equal to 'b', false otherwise.
     */
    static bool isGreaterOrEqual(double a,
                                 double b,
                                 double relEpsilon = DEFAULT_REL_EPSILON,
                                 double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return a > b || isEqual(a, b, relEpsilon, absEpsilon);
    }

    /**
     * @brief Checks if a double value is effectively zero within an absolute tolerance.
     *
     * @param a The double value to check.
     * @param absEpsilon The absolute epsilon value. Defaults to DEFAULT_ABS_EPSILON.
     * @return True if the number is considered zero, false otherwise.
     */
    static bool isZero(double a, double absEpsilon = DEFAULT_ABS_EPSILON)
    {
        return std::abs(a) < absEpsilon;
    }
};

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_COMMON_MATH
