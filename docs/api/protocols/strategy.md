# Strategy Protocol

The `Strategy` protocol (defined in `protocols/strategy.h`) is an abstract base class that defines the interface for trading strategies. Strategies are responsible for analyzing market data and emitting `common::Action` objects (buy/sell signals).

## Key Methods

-   `handleNewData(data)`: Processes new market data (e.g., from a WebSocket feed).
-   `emit()`: Returns a callable object (a function) that strategies can use to emit actions.
