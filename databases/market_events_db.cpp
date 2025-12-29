#include "market_events_db.h"
#include "bind_types.h"
#include "queries.h"

#include "../common/Accounting.h"
#include "../common/Event.h"
#include "../common/timeutils.h"

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Exception.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>

#include <spdlog/spdlog.h>

#include <optional>

#include <nlohmann/json.hpp>

namespace crypto_trader {
namespace databases {

// MarketEventsDb

// CREATORS
MarketEventsDb::MarketEventsDb(const std::string& dbPath)
: d_db(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
, d_eventQueue()
{
}

MarketEventsDb::~MarketEventsDb() { d_stopWriter = true; }

// PUBLIC MANIPULATORS
bool MarketEventsDb::init() noexcept(true)
{
    return setupPerformance() && runMigrations() && setupWriterThread();
}

bool MarketEventsDb::logEvent(const common::Event& event)
{
    // ORDER_FILLED is the only critical event
    // since you lose tracking of position,
    // tax compliance, can't recover accurate position after crash
    // This is the source of truth for position and trading.
    if (event.d_type == common::EventType::ORDER_FILLED) {
        return logEventSync(event);
    }
    return logEventAsync(event);
}

bool MarketEventsDb::logEvents(const std::vector<common::Event>& events)
{
    if (events.empty())
        return true;

    // prepare the statement
    SQLite::Statement query(d_db, SQL::insert_event);

    SQLite::Transaction transaction(d_db);

    try {
        for (const auto& event : events) {
            query.reset();

            InsertEventParams params;
            params.d_symbol    = event.d_symbol;
            params.d_qty       = event.d_qty;
            params.d_price     = event.d_price;
            params.d_type      = static_cast<int>(event.d_type);
            params.d_payload   = event.d_payload.dump();
            params.d_timestamp = event.d_timestamp;


            bind(query, params);

            // execute
            query.exec();
        }

        transaction.commit();
    }
    catch (const SQLite::Exception& e) {
        SPDLOG_ERROR("Batch insert failed: {}", e.getErrorStr());
        return false;
    }
    return true;
}

std::optional<MarketEventsDb::Events>
MarketEventsDb::getEventsSince(int64_t timestamp) noexcept(true)
{
    // pass
    SQLite::Statement query(d_db, SQL::get_events_since);

    Events events;
    try {
        GetEventsSinceParams params;
        params.d_timestamp = timestamp;
        bind(query, params);

        while (query.executeStep()) {
            common::Event event;
            event.d_symbol = query.getColumn("symbol").getString();
            event.d_qty    = query.getColumn("qty").getDouble();
            event.d_price  = query.getColumn("price").getDouble();
            event.d_type =
                static_cast<common::EventType>(query.getColumn("type").getInt());
            std::string payload = query.getColumn("payload").getString();
            event.d_payload     = nlohmann::json::parse(payload);
            event.d_timestamp   = query.getColumn("timestamp").getInt64();

            events.push_back(std::move(event));
        }
    }
    catch (const SQLite::Exception& e) {
        SPDLOG_ERROR("Failed to execute statement: {} with error: {}",
                     query.getExpandedSQL(),
                     e.getErrorStr());
        return std::nullopt;
    }

    SPDLOG_DEBUG(
        "Retrieved {} events since timestamp {}", events.size(), timestamp);
    return events;
}

std::optional<MarketEventsDb::Events> MarketEventsDb::getEventsBySymbol(
    const std::string&            symbol,
    int64_t                       startTs,
    const std::optional<int64_t>& endTs) noexcept(true)
{
    SQLite::Statement query(d_db, SQL::get_events_by_symbol);

    Events events;
    try {
        GetEventsBySymbolParams params;
        params.d_startTs = startTs;
        params.d_endTs   = endTs.value_or(common::getCurrentTimestampMs());
        params.d_symbol   = symbol;
        bind(query, params);

        while (query.executeStep()) {
            common::Event event;
            event.d_symbol = query.getColumn("symbol").getString();
            event.d_qty    = query.getColumn("qty").getDouble();
            event.d_price  = query.getColumn("price").getDouble();
            event.d_type =
                static_cast<common::EventType>(query.getColumn("type").getInt());
            std::string payload = query.getColumn("payload").getString();
            event.d_payload     = nlohmann::json::parse(payload);
            event.d_timestamp   = query.getColumn("timestamp").getInt64();

            events.push_back(std::move(event));
        }

    }
    catch (const SQLite::Exception& e) {
        SPDLOG_ERROR("Failed to execute statement: {} with error: {}",
                     query.getExpandedSQL(),
                     e.getErrorStr());
        return std::nullopt;
    }

    SPDLOG_DEBUG("Retrieved {} events between {} and {}",
                 events.size(),
                 start_ts,
                 end_ts.value_or(common::getCurrentTimestampMs()));
    return events;
}

bool MarketEventsDb::logSnapshot(const common::SymbolPositions& snapshot) noexcept(
    true)
{
    return logSnapshots({snapshot});
}

bool MarketEventsDb::logSnapshots(
    const std::vector<common::SymbolPositions>& snapshots) noexcept(true)
{

    if (snapshots.empty())
        return true;

    // prepare the statement
    SQLite::Statement query(d_db, SQL::insert_position_snapshot);

    SQLite::Statement query2(d_db, SQL::insert_snapshot_lot);

    SQLite::Transaction transaction(d_db);

    try {
        for (const auto& snapshot : snapshots) {
            query.reset();

            InsertPositionSnapshotParams params;
            params.d_symbol        = snapshot.d_symbol;
            params.d_totalQty     = snapshot.d_totalQty;
            params.d_averagePrice = snapshot.d_averagePrice;
            params.d_realizedPnl  = snapshot.d_realizedPnl;
            params.d_fifo          = snapshot.d_fifo;
            params.d_timestamp     = snapshot.d_timestamp;
            params.d_metadata      = snapshot.d_metadata.dump();

            bind(query, params);

            // execute
            query.exec();

            int64_t snapshot_id = d_db.getLastInsertRowid();

            for (const auto& lot : snapshot.d_positions_in_time) {
                query2.reset();

                InsertSnapshotLotParams lotParams;
                lotParams.d_snapshotId = snapshot_id;
                lotParams.d_totalQty   = lot.d_totalQty;
                lotParams.d_price       = lot.d_price;
                lotParams.d_timestamp   = lot.d_timestamp;

                bind(query2, lotParams);

                query2.exec();
            }
        }
        transaction.commit();
    }
    catch (const SQLite::Exception& e) {
        SPDLOG_ERROR("Batch insert failed: {}", e.getErrorStr());
        return false;
    }
    return true;
}

std::optional<common::SymbolPositions>
MarketEventsDb::getLatestSnapshot(const std::string& symbol) noexcept(true)
{
    SQLite::Statement query(d_db, SQL::get_latest_snapshot);

    try {
        GetLatestSnapshotParams params;
        params.d_symbol = symbol;
        bind(query, params);

        if (query.executeStep()) {
            int64_t         snapshot_id = query.getColumn("id").getInt64();
            common::SymbolPositions snapshot;
            snapshot.d_symbol    = query.getColumn("symbol").getString();
            snapshot.d_totalQty  = query.getColumn("total_qty").getDouble();
            snapshot.d_averagePrice =
                query.getColumn("average_price").getDouble();
            snapshot.d_realizedPnl =
                query.getColumn("realized_pnl").getDouble();

            SQLite::Statement innerQuery(d_db, SQL::get_snapshot_lots);

            GetSnapshotLotsParams innerParams;
            innerParams.d_snapshotId = snapshot_id;
            bind(innerQuery, innerParams);

            std::list<common::Position> positions;
            while (innerQuery.executeStep()) {
                positions.emplace_back();
                auto& p       = positions.back();
                p.d_totalQty = innerQuery.getColumn("total_qty").getDouble();
                p.d_price     = innerQuery.getColumn("price").getDouble();
                p.d_timestamp = innerQuery.getColumn("timestamp").getInt();
            }

            snapshot.d_positions_in_time = std::move(positions);

            return snapshot;
        }
        return std::nullopt;
    }
    catch (const SQLite::Exception& e) {
        SPDLOG_ERROR("Failed to execute statement: {} with error: {}",
                     query.getExpandedSQL(),
                     e.getErrorStr());
        return std::nullopt;
    }
}

std::optional<MarketEventsDb::SymbolPositionsVec>
MarketEventsDb::getLatestSnapshots() noexcept(true)
{
    SQLite::Statement query(d_db, SQL::get_latest_snapshots);

    SymbolPositionsVec snapshots;
    try {
        while (query.executeStep()) {
            int64_t         snapshot_id = query.getColumn("id").getInt64();
            common::SymbolPositions snapshot;
            snapshot.d_symbol    = query.getColumn("symbol").getString();
            snapshot.d_totalQty  = query.getColumn("total_qty").getDouble();
            snapshot.d_averagePrice =
                query.getColumn("average_price").getDouble();
            snapshot.d_realizedPnl =
                query.getColumn("realized_pnl").getDouble();
            snapshot.d_timestamp = query.getColumn("timestamp").getInt64();
            snapshot.d_metadata =
                nlohmann::json::parse(query.getColumn("metadata").getString());

            SQLite::Statement innerQuery(d_db, SQL::get_snapshot_lots);

            GetSnapshotLotsParams innerParams;
            innerParams.d_snapshotId = snapshot_id;
            bind(innerQuery, innerParams);

            std::list<common::Position> positions;
            while (innerQuery.executeStep()) {
                positions.emplace_back();
                auto& p       = positions.back();
                p.d_totalQty = innerQuery.getColumn("total_qty").getDouble();
                p.d_price     = innerQuery.getColumn("price").getDouble();
                p.d_timestamp = innerQuery.getColumn("timestamp").getInt();
            }

            snapshot.d_positions_in_time = std::move(positions);

            snapshots.push_back(std::move(snapshot));
        }
    }
    catch (const SQLite::Exception& e) {
        SPDLOG_ERROR("Failed to execute statement: {} with error: {}",
                     query.getExpandedSQL(),
                     e.getErrorStr());
        return std::nullopt;
    }

    return snapshots;
}

// PRIVATE MANIPULATORS
bool MarketEventsDb::runMigrations() noexcept(true)
{
    // 32-bit integer stored directly in the SQLite header. Piece of
    // metadata that persists between runs, making it perfect for tracking
    // with migration the database is on.
    const int currentVersion = d_db.execAndGet("PRAGMA user_version").getInt();

    // start a transaction for speed and safety
    SQLite::Transaction transaction(d_db);

    if (currentVersion < 1) {
        SPDLOG_INFO("Migrating to version 1: Creating events table...");
        // execute the schema strings
        try {
            SPDLOG_INFO("Creating events table...");
            d_db.exec(SQL::events);
            SPDLOG_INFO("Creating snapshots table...");
            d_db.exec(SQL::position_snapshots);
            SPDLOG_INFO("Creating snapshots_lots tables...");
            d_db.exec(SQL::snapshot_lots);
        }
        catch (const SQLite::Exception& e) {
            SPDLOG_ERROR(
                "Unable to execute initial sql expressions for events "
                "with error {}",
                e.getErrorStr());
            return false;
        }

        d_db.exec("PRAGMA user_version = 1");
    }

    transaction.commit();

    return true;
}

void MarketEventsDb::writerThreadLoop() noexcept(true)
{
    static constexpr int BATCH_SIZE = 100;
    std::vector<common::Event>   batch;
    batch.reserve(BATCH_SIZE);

    common::Event event;
    while (!d_stopWriter) {

        while (batch.size() < BATCH_SIZE && d_eventQueue.pop(event)) {
            batch.push_back(std::move(event));
        }


        if (!batch.empty()) {
            logEvents(batch);
            batch.clear();
        }
        else {
            // Sleep briefly
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool MarketEventsDb::setupWriterThread()
{
    assert(!d_stopWriter &&
           "attempt to setup writer thread that has already started");
    SPDLOG_INFO("Setting up MarketEventsDb writer thread...");
    d_writerThread =
        std::jthread([this](std::stop_token st) { writerThreadLoop(); });
    return true;
}

bool MarketEventsDb::setupPerformance() noexcept(true)
{
    try {

        // 1. Enable WAL mode (This only needs to be run once per DB, but
        // safe to repeat)
        d_db.exec("PRAGMA journal_mode = WAL");

        // 2. Set Synchronous to NORMAL for the best performance in WAL
        // mode
        d_db.exec("PRAGMA synchronous = NORMAL");
    }
    catch (const SQLite::Exception& e) {
        SPDLOG_ERROR("Unable to setup core performance configuration changes "
                     "for sqlite with error: {}",
                     e.getErrorStr());
        return false;
    }
    return true;
}

bool MarketEventsDb::logEventSync(const common::Event& event)
{
    return logEvents({event});
}

bool MarketEventsDb::logEventAsync(const common::Event& event)
{
    if (!d_eventQueue.push(event)) {

        SPDLOG_WARN("Event queue full, dropping event...");
        return false;
    }
    return true;
}

} // namespace databases
} // namespace crypto_trader
