CREATE TABLE IF NOT EXISTS events (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    symbol      TEXT NOT NULL,
    qty         REAL NOT NULL,
    price       REAL NOT NULL,
    type        INTEGER NOT NULL,
    payload     TEXT,
    timestamp   INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_events_symbol_ts 
ON events(symbol, timestamp);

CREATE INDEX IF NOT EXISTS idx_events_type_ts
ON events(type, timestamp);

CREATE INDEX IF NOT EXISTS idx_events_symbol_type_ts
ON events(symbol, type, timestamp);
