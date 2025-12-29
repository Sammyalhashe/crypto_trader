CREATE TABLE IF NOT EXISTS snapshot_lots (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    snapshot_id INTEGER NOT NULL,
    total_qty   REAL NOT NULL,
    price       REAL NOT NULL,
    timestamp   INTEGER NOT NULL,
    FOREIGN KEY (snapshot_id) REFERENCES position_snapshots(id)
);

CREATE INDEX IF NOT EXISTS idx_snapshot_lots ON snapshot_lots(snapshot_id);
