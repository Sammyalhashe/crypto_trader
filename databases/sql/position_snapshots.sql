CREATE TABLE IF NOT EXISTS position_snapshots (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    symbol          TEXT NOT NULL,
    total_qty       REAL NOT NULL,
    average_price   REAL NOT NULL,
    fifo            BOOLEAN NOT NULL,
    realized_pnl    REAL NOT NULL,
    timestamp       INTEGER NOT NULL,
    metadata        TEXT
);

CREATE INDEX IF NOT EXISTS idx_snapshots_symbol_ts
ON position_snapshots(symbol, timestamp DESC);
