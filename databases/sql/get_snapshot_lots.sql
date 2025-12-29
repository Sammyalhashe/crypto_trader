SELECT total_qty, price, timestamp 
FROM snapshot_lots 
WHERE snapshot_id = :snapshot_id
ORDER BY timestamp ASC;
