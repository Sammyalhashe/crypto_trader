# Executor Protocol

The `Executor` protocol (defined in `protocols/executor.h`) is an abstract base class that defines the interface for executing trades. Concrete implementations, such as `PaperTradingExecutor` and `RealTradingExecutor`, provide different execution behaviors.

## Key Methods

-   `buy(product, quantity)`: Places a buy order for a specified quantity of a product.
-   `sell(product, quantity)`: Places a sell order for a specified quantity of a product.
-   `getBalance(currency)`: Retrieves the current balance for a given currency.
-   `getPosition(product)`: Retrieves the current position for a given product.
-   `processTickerData(product, price, timestamp)`: Updates internal state with new market data.
