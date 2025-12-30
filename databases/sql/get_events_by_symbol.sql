SELECT id, symbol, qty, price, type, payload, timestamp 
FROM events 
WHERE timestamp >= :start_ts AND timestamp <= :end_ts AND symbol = :symbol;
