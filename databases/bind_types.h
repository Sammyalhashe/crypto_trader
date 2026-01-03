#ifndef INCLUDED_CRYPTO_TRADER_DATABASES_BIND_TYPES
#define INCLUDED_CRYPTO_TRADER_DATABASES_BIND_TYPES

#include <SQLiteCpp/Statement.h>
#include <string>

namespace crypto_trader {
namespace databases {

struct InsertEventParams {
    std::string d_symbol;
    double      d_qty;
    double      d_price;
    int         d_type;
    std::string d_payload;
    int64_t     d_timestamp;
};

inline void bind(SQLite::Statement& query, const InsertEventParams& params)
{
    query.bind(":symbol", params.d_symbol);
    query.bind(":qty", params.d_qty);
    query.bind(":price", params.d_price);
    query.bind(":type", params.d_type);
    query.bind(":payload", params.d_payload);
    query.bind(":timestamp", params.d_timestamp);
}

struct GetEventsSinceParams {
    int64_t d_timestamp;
};

inline void bind(SQLite::Statement& query, const GetEventsSinceParams& params)
{
    query.bind(":timestamp", params.d_timestamp);
}

struct GetEventsBySymbolParams {
    int64_t     d_startTs;
    int64_t     d_endTs;
    std::string d_symbol;
};

inline void bind(SQLite::Statement&             query,
                 const GetEventsBySymbolParams& params)
{
    query.bind(":start_ts", params.d_startTs);
    query.bind(":end_ts", params.d_endTs);
    query.bind(":symbol", params.d_symbol);
}

struct InsertPositionSnapshotParams {
    std::string d_symbol;
    double      d_totalQty;
    double      d_averagePrice;
    double      d_realizedPnl;
    bool        d_fifo;
    int64_t     d_timestamp;
    std::string d_metadata;
};

inline void bind(SQLite::Statement&                  query,
                 const InsertPositionSnapshotParams& params)
{
    query.bind(":symbol", params.d_symbol);
    query.bind(":total_qty", params.d_totalQty);
    query.bind(":average_price", params.d_averagePrice);
    query.bind(":realized_pnl", params.d_realizedPnl);
    query.bind(":fifo", params.d_fifo);
    query.bind(":timestamp", params.d_timestamp);
    query.bind(":metadata", params.d_metadata);
}

struct InsertSnapshotLotParams {
    int64_t d_snapshotId;
    double  d_totalQty;
    double  d_price;
    int64_t d_timestamp;
};

inline void bind(SQLite::Statement&             query,
                 const InsertSnapshotLotParams& params)
{
    query.bind(":snapshot_id", params.d_snapshotId);
    query.bind(":total_qty", params.d_totalQty);
    query.bind(":price", params.d_price);
    query.bind(":timestamp", params.d_timestamp);
}

struct GetLatestSnapshotParams {
    std::string d_symbol;
};

inline void bind(SQLite::Statement&             query,
                 const GetLatestSnapshotParams& params)
{
    query.bind(":symbol", params.d_symbol);
}

struct GetSnapshotLotsParams {
    int64_t d_snapshotId;
};

inline void bind(SQLite::Statement& query, const GetSnapshotLotsParams& params)
{
    query.bind(":snapshot_id", params.d_snapshotId);
}

} // namespace databases
} // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_DATABASES_BIND_TYPES
