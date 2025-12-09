# Floating Point Comparisons

In floating-point arithmetic, numbers that should be theoretically equal often differ slightly due to precision limits. This is known as the "picket fence" effect: representable numbers are discrete, not continuous.

## The Problem with Fixed Epsilon

Using a fixed small number (like `1e-9`) as an error margin (epsilon) works for numbers near 1.0 but fails at extremes:

1.  **Tiny Numbers**: For a price of `1e-10`, a fixed epsilon of `1e-9` is huge (1000% error margin). Two very different tiny numbers will look "equal".
2.  **Huge Numbers**: For a market cap of `1e12`, the machine precision error might be `1e-4`. A fixed epsilon of `1e-9` is too tight, and theoretically equal numbers will look "unequal" due to noise.

## The Solution: Hybrid Comparison

We use a hybrid approach that combines **Relative** and **Absolute** comparisons:

$$ |a - b| \le \max(\text{absEpsilon}, \text{relEpsilon} \times \max(|a|, |b|)) $$

If $\epsilon$ is `1e-9`, you are effectively asking: **"Do these numbers agree to 9 significant digits?"**

*   **Crypto Example (High Price):** BTC at 100,000.
    *   Allowed error = $100,000 \times 10^{-9} = 0.0001$.
    *   This scales appropriately for the asset class.
*   **Crypto Example (Low Price):** SHIB at 0.00001.
    *   Allowed error = $0.00001 \times 10^{-9} = 10^{-14}$.
    *   This tightens the requirement, ensuring we don't treat a 50% price difference as "equal."

### Example Code

```cpp
bool isEqual(double a, double b, double relEpsilon = 1e-9, double absEpsilon = 1e-12) {
    double diff = std::abs(a - b);
    if (diff < absEpsilon) return true; // Absolute check for near-zero
    return diff <= (std::max(std::abs(a), std::abs(b)) * relEpsilon); // Relative check
}
```

