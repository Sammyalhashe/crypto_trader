# Math Utilities

The `Math` class (defined in `common/math.h`) provides static utility methods for safe floating-point comparisons. These methods use a hybrid approach combining relative and absolute epsilon values to account for the inherent imprecision of floating-point numbers across different scales.

## Key Methods

-   `isEqual(a, b, relEpsilon, absEpsilon)`: Checks if two doubles are considered equal within a specified tolerance.
-   `isNotEqual(a, b, relEpsilon, absEpsilon)`: Checks if two doubles are considered not equal.
-   `isLess(a, b, relEpsilon, absEpsilon)`: Checks if `a` is strictly less than `b` (outside the equality zone).
-   `isGreater(a, b, relEpsilon, absEpsilon)`: Checks if `a` is strictly greater than `b` (outside the equality zone).
-   `isLessOrEqual(a, b, relEpsilon, absEpsilon)`: Checks if `a` is less than or equal to `b`.
-   `isGreaterOrEqual(a, b, relEpsilon, absEpsilon)`: Checks if `a` is greater than or equal to `b`.
-   `isZero(a, absEpsilon)`: Checks if a double is effectively zero.

For a detailed explanation of the hybrid comparison approach, refer to the [Floating Point Comparisons](../../learning/numerics/floating_point_comparisons.md) learning document.