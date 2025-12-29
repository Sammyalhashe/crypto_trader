#ifndef INCLUDED_CRYPTO_TRADER_DATABASES_BIND_TYPES
#define INCLUDED_CRYPTO_TRADER_DATABASES_BIND_TYPES

#include <SQLiteCpp/Statement.h>
#include <string>

namespace crypto_trader {
namespace databases {

struct insert_event_params {
    std::string symbol;
    double qty;
    double price;
    int type;
    std::string payload;
    int64_t timestamp;
};

inline void bind(SQLite::Statement& query, const insert_event_params& params) {
    query.bind(":symbol", params.symbol);
    query.bind(":qty", params.qty);
    query.bind(":price", params.price);
    query.bind(":type", params.type);
    query.bind(":payload", params.payload);
    query.bind(":timestamp", params.timestamp);
}

struct get_events_since_params {
    int64_t timestamp;
};

inline void bind(SQLite::Statement& query, const get_events_since_params& params) {
    query.bind(":timestamp", params.timestamp);
}

struct get_events_by_symbol_params {
    int64_t start_ts;
    int64_t end_ts;
    std::string symbol;
};

inline void bind(SQLite::Statement& query, const get_events_by_symbol_params& params) {
    query.bind(":start_ts", params.start_ts);
    query.bind(":end_ts", params.end_ts);
    query.bind(":symbol", params.symbol);
}

struct insert_position_snapshot_params {
    std::string symbol;
    double total_qty;
    double average_price;
    double realized_pnl;
    bool fifo;
    int64_t timestamp;
    std::string metadata;
};

inline void bind(SQLite::Statement& query, const insert_position_snapshot_params& params) {
    query.bind(":symbol", params.symbol);
    query.bind(":total_qty", params.total_qty);
    query.bind(":average_price", params.average_price);
    query.bind(":realized_pnl", params.realized_pnl);
    query.bind(":fifo", params.fifo);
    query.bind(":timestamp", params.timestamp);
    query.bind(":metadata", params.metadata);
}

struct insert_snapshot_lot_params {
    int64_t snapshot_id;
    double total_qty;
    double price;
    int64_t timestamp;
};

inline void bind(SQLite::Statement& query, const insert_snapshot_lot_params& params) {
    query.bind(":snapshot_id", params.snapshot_id);
    query.bind(":total_qty", params.total_qty);
    query.bind(":price", params.price);
    query.bind(":timestamp", params.timestamp);
}

struct get_latest_snapshot_params {
    std::string symbol;
};

inline void bind(SQLite::Statement& query, const get_latest_snapshot_params& params) {
    query.bind(":symbol", params.symbol);
}

struct get_snapshot_lots_params {
    int64_t snapshot_id;
};

inline void bind(SQLite::Statement& query, const get_snapshot_lots_params& params) {
    query.bind(":snapshot_id", params.snapshot_id);
}

} // namespace databases
} // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_DATABASES_BIND_TYPES
