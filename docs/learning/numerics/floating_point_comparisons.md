# Floating Point Comparisons

In floating-point arithmetic, numbers that should be theoretically equal often differ slightly due to precision limits. This is known as the "picket fence" effect: representable numbers are discrete, not continuous.

## The Problem with Fixed Epsilon

Using a fixed small number (like `1e-9`) as an error margin (epsilon) works for numbers near 1.0 but fails at extremes:

1.  **Tiny Numbers**: For a price of `1e-10`, a fixed epsilon of `1e-9` is huge (1000% error margin). Two very different tiny numbers will look "equal".
2.  **Huge Numbers**: For a market cap of `1e12`, the machine precision error might be `1e-4`. A fixed epsilon of `1e-9` is too tight, and theoretically equal numbers will look "unequal" due to noise.

## The Solution: Hybrid Comparison

We use a hybrid approach that combines **Relative** and **Absolute** comparisons:

$$ |a - b| \le \max(\text{absEpsilon}, \text{relEpsilon} \times \max(|a|, |b|)) $$

1.  **Near Zero**: If the difference is smaller than `absEpsilon` (e.g., `1e-12`), they are equal. This handles the "empty account" case where relative comparison would fail (0 vs 0).
2.  **Larger Numbers**: We check if the difference is within a percentage (`relEpsilon`) of the larger magnitude. This scales the error margin with the data.

### Example Code

```cpp
bool isEqual(double a, double b, double relEpsilon = 1e-9, double absEpsilon = 1e-12) {
    double diff = std::abs(a - b);
    if (diff < absEpsilon) return true; // Absolute check for near-zero
    return diff <= (std::max(std::abs(a), std::abs(b)) * relEpsilon); // Relative check
}
```

```