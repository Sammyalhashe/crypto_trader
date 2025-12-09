# HODL Strategy

The `HodlStrategy` (defined in `strategies/hodl.h`) is a simple trading strategy that implements a "Buy and Hold" approach with dynamic adjustments. It aims to buy on dips and potentially sell on significant gains.

## Configuration

Configured via `HodlStrategyConfig`, which includes parameters like:
-   `percentUp`: Percentage gain to trigger a sell.
-   `percentDown`: Percentage dip to trigger a buy.
-   `buyAmount`: The amount of base currency to use for buy orders.
-   `initStrategy`: Initial behavior (e.g., `e_BUY_IMMEDIATELY` or `e_SET_BASIS_PRICE`).

## Logic

-   Analyzes ticker data to determine price movements.
-   Emits buy or sell actions based on configured thresholds and current position.
-   Tracks `lastBuyPrice` and `hasBoughtAgain` to manage its state.
