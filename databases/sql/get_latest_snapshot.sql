SELECT id, symbol, total_qty, average_price, realized_pnl, fifo, timestamp, metadata 
FROM position_snapshots 
WHERE symbol = :symbol
ORDER BY timestamp DESC 
LIMIT 1;
