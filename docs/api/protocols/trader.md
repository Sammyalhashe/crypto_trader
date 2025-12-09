# Trader Protocol

The `Trader` protocol (defined in `protocols/trader.h`) is an abstract base class that defines the overall trading process. It orchestrates interactions between WebSocket clients, strategies, and executors.

## Key Methods

-   `listen(buffer)`: Processes raw data received from a WebSocket client.
