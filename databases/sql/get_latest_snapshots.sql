SELECT id, symbol, total_qty, average_price, realized_pnl, fifo, timestamp, metadata 
FROM position_snapshots ps1 
WHERE timestamp = (SELECT MAX(timestamp) FROM position_snapshots ps2 WHERE ps1.symbol = ps2.symbol);
