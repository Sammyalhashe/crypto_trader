# Crypto Trader

A high-performance cryptocurrency trading bot written in C++20, featuring real-time WebSocket integration, flexible strategy implementation, and hybrid paper/real trading execution modes. It leverages Zig for specific performance-critical components.

## Features

-   **Hybrid Execution**: Seamlessly switch between Paper Trading (simulated) and Real Trading.
-   **Exchange Integration**: Native support for Coinbase Advanced Trade WebSockets.
-   **Strategies**:
    -   **HODL**: Configurable buy/sell logic based on percentage dips and gains.
    -   Extensible `Strategy` interface for custom logic.
-   **Performance**:
    -   C++20 codebase.
    -   Zig integration for optimized math and order book operations.
    -   Asynchronous I/O using Boost.Asio.
-   **Data Management**: Local storage of market data sequence and ticker history.

## Project Structure

-   `adaptors/`: Exchange connectivity (WebSocket clients).
-   `common/`: Shared utilities (Math, JSON, Time, Serialization).
-   `executors/`: Order execution logic (Paper vs. Real).
-   `strategies/`: Trading algorithms (HODL, Simple).
-   `traders/`: Core orchestration connecting adaptors, strategies, and executors.
-   `zig/`: Zig modules for performance-critical math and data structures.

## Development

This project uses **Nix** for a reproducible development environment.

### Setup
```bash
nix develop
```

### Build & Test
Inside the Nix shell:
1.  **Configure**: `prepare` (runs CMake).
2.  **Build**: `build` (compiles the project).
3.  **Test**: `make test` (runs GoogleTest suite). *Note: Run `build` before testing if source changed.*

## Learnings

Documentation on technical concepts and design decisions:

-   [Floating Point Comparisons](docs/learning/numerics/floating_point_comparisons.md): Why and how we use hybrid epsilon comparisons for currency math.
