SELECT id, symbol, qty, price, type, payload, timestamp 
FROM events 
WHERE timestamp >= :timestamp;
