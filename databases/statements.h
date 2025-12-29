#ifndef INCLUDED_CRYPTO_TRADER_DATABASES_STATMENTS
#define INCLUDED_CRYPTO_TRADER_DATABASES_STATMENTS

namespace crypto_trader {
namespace databases {

// inline DATA
inline constexpr const char *CREATE_EVENTS_TABLE = R"(
CREATE TABLE IF NOT EXISTS events (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    symbol      TEXT NOT NULL,
    qty         REAL NOT NULL,
    price       REAL NOT NULL,
    type        INTEGER NOT NULL,
    payload     TEXT,
    timestamp   INTEGER NOT NULL
);
)";

inline constexpr const char *CREATE_INDEX_SYMBOL_TS = R"(
CREATE INDEX IF NOT EXISTS idx_events_symbol_ts 
ON events(symbol, timestamp);
)";

inline constexpr const char *CREATE_INDEX_TYPE_TS = R"(
CREATE INDEX IF NOT EXISTS idx_events_type_ts
ON events(type, timestamp);
)";

inline constexpr const char *CREATE_INDEX_SYMBOL_TYPE_TS = R"(
CREATE INDEX IF NOT EXISTS idx_events_symbol_type_ts
ON events(symbol, type, timestamp);
)";

inline constexpr const char *CREATE_SNAPSHOTS_TABLE = R"(
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
)";

inline constexpr const char *CREATE_INDEX_SNAPSHOTS = R"(
CREATE INDEX IF NOT EXISTS idx_snapshots_symbol_ts
ON position_snapshots(symbol, timestamp DESC);
)";

inline constexpr const char *CREATE_SNAPSHOT_LOTS_TABLE = R"(
CREATE TABLE IF NOT EXISTS snapshot_lots (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    snapshot_id INTEGER NOT NULL,
    total_qty   REAL NOT NULL,
    price       REAL NOT NULL,
    timestamp   INTEGER NOT NULL,
    FOREIGN KEY (snapshot_id) REFERENCES position_snapshots(id)
);
)";

inline constexpr const char *CREATE_INDEX_SNAPSHOT_LOTS = R"(
CREATE INDEX IF NOT EXISTS idx_snapshot_lots ON snapshot_lots(snapshot_id);
)";

}; // namespace databases
}; // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_DATABASES_STATMENTS
