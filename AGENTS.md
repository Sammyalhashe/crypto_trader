 #AGENTS.md - Codebase Documentation for AI Agents

This document provides essential information for AI agents working in the `crypto_trader` repository.

## 1. Project Overview

This project is a C++-based cryptocurrency trading bot that integrates with exchange WebSockets, implements various trading strategies, and supports both paper trading and real trading (with a placeholder for live execution). It leverages Zig for potential performance-critical components.

## 2. Build and Development Environment

The project uses a Nix flake for reproducible builds and development environment setup.

-   **Activate Development Environment**:
    ```bash
    nix develop
    ```
    This command will set up the necessary dependencies and make the following commands available in your shell:
    -   `prepare`: Configures the project (likely CMake).
    -   `build`: Compiles the project.
        -   `make test`: Runs the project's tests.. **Note**: This command does *not* rebuild the code. Always run `build` before testing if you have modified source files.
                        -   **Approved Commands**: You are explicitly allowed and encouraged to use `prepare`, `build`, `make test`, `git` (status, add, commit, push, diff, remote, ls-files, rm), `clang-format`, `ls`, `cat`, `rm`, `mkdir`, `touch`, `find`, `grep`, `echo` and `nvim` to manage the project lifecycle.

-   **`adaptors/`**: Contains client implementations for interacting with external services, primarily WebSocket clients for exchanges (e.g., `coinbase_websocket_client`).
-   **`cmake/`**: CMake utility files for compiler warnings, Doxygen, static analyzers, and common functions.
-   **`common/`**: General utility functions, JSON utilities (`jsonutils.h`), file utilities (`fileutils.h`), serialization, and common data types (`types.h`).
-   **`databases/`**: Implementations for storing market data (e.g., `market_data_db.h`).
-   **`doc/`**: Documentation files.
-   **`executors/`**: Contains the trade execution logic.
    -   `paper_trading_executor.h`/`.cpp`: Implements simulated trading logic, managing balances and holdings for paper trading. Configurable with `PaperTradingExecutorConfig` for initial balance, commission rate, product, and initial strategy (buy immediately or set basis price).
    -   `real_trading_executor.h`/`.cpp`: A placeholder for live trade execution. Currently, it logs actions but does not interact with a real exchange.
-   **`protocols/`**: Defines interfaces (abstract base classes) for core components like `Executor`, `Strategy`, `Trader`, and `WebsocketClient`.
-   **`strategies/`**: Implementations of various trading strategies (e.g., `hodl.h`/`.cpp`). Strategies emit `common::Action` objects.
-   **`traders/`**: Manages the overall trading process, including connecting to exchanges, running strategies, and executing trades via an executor.
    -   `CoinbaseTrader.h`/`.cpp`: The main trader implementation for Coinbase. It can be configured for paper trading via the `paperTrading` flag in `CoinbaseTraderConfig`. It dynamically instantiates either a `PaperTradingExecutor` or `RealTradingExecutor` based on this flag.
-   **`zig/`**: Contains Zig language code, including `zigmath` and `order_book`. This suggests that some performance-critical parts of the system, such as mathematical operations or order book logic, might be implemented in Zig and integrated with the C++ codebase.

## 4. Key Concepts and Patterns

### 4.1 Executors

The `Executor` (`protocols/executor.h`) is a key abstraction for trade execution.
-   `PaperTradingExecutor` simulates trades locally.
-   `RealTradingExecutor` is the interface for real-world exchange interactions.

### 4.2 Traders

The `Trader` (`protocols/trader.h`) orchestrates the trading process. A concrete implementation like `CoinbaseTrader` connects to a specific exchange, runs a configured strategy, and uses an `Executor` to perform trades.

### 4.3 Paper Trading Configuration

`CoinbaseTraderConfig` includes a `paperTrading` boolean flag.
-   If `paperTrading` is `true`, `CoinbaseTrader` will instantiate and use a `PaperTradingExecutor`.
-   The `PaperTradingExecutorConfig` is populated from the `strategyConfig` JSON passed to the `CoinbaseTraderConfig`, allowing for per-trader paper trading configurations (e.g., initial balance, commission, and initial strategy).

### 4.4 Strategy Integration

Strategies (e.g., `HodlStrategy`) are responsible for generating `common::Action` (buy/sell) based on market data. These actions are then passed to the `Trader`, which in turn delegates them to the configured `Executor`.

### 4.5 Market Data Flow

Market data (e.g., ticker updates) flows from the `WebsocketClient` to the `Trader`'s `handleNewData` method. The `Trader` then passes this data to its `d_strategy` and `d_executor` to update their internal states and potentially trigger actions.

## 5. Coding Conventions and Style

-   **C++ Formatting**: A `.clang-format` file exists in the project root. You **must** verify that all C++ code edits conform to these rules. Run `clang-format -i ` (if available) or check the `.clang-format` configuration before finalizing changes.
-   **Logging**: `spdlog` is used for logging throughout the codebase. Use `SPDLOG_INFO`, `spdlog::warn`, `spdlog::error` as appropriate.
-   **JSON Handling**: `nlohmann::json` is used for JSON parsing and manipulation (`common/jsonutils.h` provides utility functions).
-   **Error Handling**: Assertions (`assert(false)`) are used for unrecoverable errors, and `spdlog::error` for runtime errors.

## 6. Gotchas and Non-Obvious Patterns

-   **Nix Flake**: Ensure you are in a `nix develop` shell to have the build and test commands available.
-   **Executor Management**: The `CoinbaseTrader` dynamically manages its `Executor` based on configuration. Be aware of which executor is active when debugging or extending functionality.
-   **Market Data Timestamping**: In `PaperTradingExecutor`, trade timestamps are currently updated in `processTickerData` when empty. This might be a simplification and could need refinement for more precise historical trade tracking.
-   **Live Trading Stubs**: The `RealTradingExecutor` and the non-paper trading path in `CoinbaseTrader::handleAction` are currently stubs. Expanding these would involve integrating with actual exchange APIs for order placement and account management.

## 7. Editor Instructions
-   **Neovim**: If you detect that the neovim instance is listening on `/tmp/nvimtrader`, perform all file edits and diffs through that instance. Use a dedicated tab for your agent operations.
    1.  **Create/Focus Tab**: Check if a tab with var `agent_tab` exists. If not, create it.
        2.  **Label Tab**: Set the tab variable `t:agent_tab` to identify it.
                        3.  **Send Commands**: Use `nvim --server /tmp/nvimtrader --remote-send` to send commands. **Crucially**, prefix commands with `:tabnext
    ` or ensure you are operating within that tab to avoid interfering with the user's active buffer.


## 8. Git Best Practices
-   **Complex Commit Messages**: To avoid bash syntax errors (e.g., with newlines or special characters), write the commit message to a temporary file first, use `git commit -F <file>`, and then **remove the temporary file**.
    ```bash
    cat <<EOF > commit_msg.txt
    Title

    Body.
    EOF
    git commit -F commit_msg.txt
    rm commit_msg.txt
    ```


## 9. Git Remotes Syncing
-   **Multi-Remote Push**: The main remote is GitHub (often named `upstream`). You **must** push to it.
-   **Syncing**: If you detect a Gitea remote (e.g., matching the `picloud.*` domain, often named `origin`), you **must** push to that remote as well to ensure synchronization.

