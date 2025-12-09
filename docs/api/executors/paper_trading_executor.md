# Paper Trading Executor

The `PaperTradingExecutor` (defined in `executors/paper_trading_executor.h`) is a concrete implementation of the `Executor` protocol. It simulates trade execution locally, managing an internal balance and positions without interacting with real exchanges.

## Configuration

It is configured via `PaperTradingExecutorConfig`, which allows setting an initial balance, commission rates, and other simulation parameters.

## Behavior

-   Simulates buys and sells, adjusting balances and positions.
-   Calculates simulated commissions and updates realized PnL.
-   Relies on `EventPositionManager` to track positions.
