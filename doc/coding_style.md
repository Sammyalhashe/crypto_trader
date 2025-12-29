# Coding Style Guide

This document outlines the coding style and conventions enforced in the `crypto_trader` project. Adherence to these styles ensures consistency, readability, and high code quality across the codebase.

## 1. General Principles

-   **Consistency**: Follow the existing patterns in the codebase.
-   **Modern C++**: Use C++20/23 features where appropriate.
-   **Safety**: Prefer `std::optional`, `std::variant`, and smart pointers over raw pointers and manual memory management.

## 2. Naming Conventions

The project uses a specific naming convention that is partially enforced by a custom script (`style_checker.py`).

### 2.1 Variables

-   **Local Variables**: Use `camelCase`.
    ```cpp
    int eventCount = 0;
    std::string productSymbol = "BTC-USD";
    ```
-   **Member Variables**: Use the `d_` prefix followed by `camelCase`.
    ```cpp
    class MyClass {
      private:
        int d_count;
        std::string d_symbol;
    };
    ```
-   **Private Pointer Members**: Use the `_p` suffix for private member pointers.
    ```cpp
    class MyClass {
      private:
        std::shared_ptr<Database> d_db_p;
    };
    ```

### 2.2 Functions and Methods

-   **Method Names**: Use `camelCase`.
    ```cpp
    void applyEvent(const Event& e);
    ```

### 2.3 Classes and Structs

-   **Class/Struct Names**: Use `PascalCase`.
    ```cpp
    class CoinbaseTrader;
    struct SymbolPositions;
    ```

## 3. Formatting

We use `clang-format` to enforce consistent indentation, spacing, and line breaks. A `.clang-format` file is provided in the project root.

-   **Indentation**: 4 spaces.
-   **Braces**: K&R style (braces on the same line as the statement).

You can format the entire project using:
```bash
make format
```

## 4. Static Analysis

We use `cppcheck` and custom scripts to catch common errors and style violations.

### 4.1 Enforced Checks

-   **Unused Variables**: Warns about variables that are declared but never used.
-   **Missing const Qualifiers**: Encourages the use of `const` for parameters and methods that do not modify state.
-   **Naming Conventions**: Enforced by `style_checker.py`.

### 4.2 Running Checks

To run the style and static analysis checks:
```bash
make style-check
```
This target runs both `cppcheck` and the naming convention script.

## 5. Documentation

-   Use Doxygen-style comments (`/** ... */` or `/// ...`) for all public headers and methods.
-   Document parameters using `@param` and return values using `@return`.
-   Document the *why* rather than the *what* for complex logic.
