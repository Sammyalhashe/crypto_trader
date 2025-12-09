# Accounting

The `Accounting` class (defined in `common/Accounting.h`) manages the financial records for trading activities, tracking positions, average prices, and realized profit and loss (PnL). It processes trade events and provides snapshots of the current state.

## Key Methods

-   `apply_event(event)`: Processes a new trade event to update positions.
-   `snapshot()`: Returns a snapshot of all current positions.
-   `replay_events(events)`: Replays a series of historical events to reconstruct the accounting state.
