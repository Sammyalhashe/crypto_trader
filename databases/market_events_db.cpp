#include "market_events_db.h"

#include "../common/Event.h"
#include "../common/timeutils.h"

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Exception.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <optional>

#include <nlohmann/json.hpp>
#include <utility>

namespace crypto_trader {
namespace databases {

// STATIC DATA
static constexpr const char *CREATE_EVENTS_TABLE = R"(
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

static constexpr const char *CREATE_INDEX_SYMBOL_TS = R"(
CREATE INDEX IF NOT EXISTS idx_events_symbol_ts 
ON events(symbol, timestamp);
)";

static constexpr const char *CREATE_INDEX_TYPE_TS = R"(
CREATE INDEX IF NOT EXIST idx_events_type_ts
ON events(type, timestamp);
)";

static constexpr const char *CREATE_INDEX_SYMBOL_TYPE_TS = R"(
CREATE INDEX IF NOT EXIST idx_events_symbol_type_ts
ON events(symbol, type, timestamp);
)";

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

bool MarketEventsDb::logEvent(const Event& event)
{
    // ORDER_FILLED is the only critical event
    // since you lose tracking of position,
    // tax compliance, can't recover accurate position after crash
    // This is the source of truth for position and trading.
    if (event.d_type == EventType::ORDER_FILLED) {
        return logEventSync(event);
    }
    return logEventAsync(event);
}

bool MarketEventsDb::logEvents(const std::vector<Event>& events)
{
    if (events.empty())
        return true;

    // prepare the statement
    SQLite::Statement query(
        d_db,
        "INSERT INTO events (symbol, qty, price, type, payload, timestamp)"
        "VALUES (?, ?, ?, ?, ?, ?)");

    SQLite::Transaction transaction(d_db);

    try {
        for (const auto& event : events) {
            query.reset();
            // bind the values
            query.bind(1, event.d_symbol);
            query.bind(2, event.d_qty);
            query.bind(3, event.d_price);
            query.bind(4, static_cast<int>(event.d_type));
            query.bind(5, event.d_payload.dump());
            query.bind(6, event.d_timestamp);

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
    SQLite::Statement query(d_db,
                            "SELECT id, symbol, qty, price, type, payload, "
                            "timestamp FROM events WHERE timestamp >= ?");

    Events events;
    try {
        query.bind(1, timestamp);

        while (query.executeStep()) {
            Event event;
            event.d_symbol = query.getColumn(1).getString();
            event.d_qty    = query.getColumn(2).getDouble();
            event.d_price  = query.getColumn(3).getDouble();
            event.d_type = static_cast<EventType>(query.getColumn(4).getInt());
            std::string payload = query.getColumn(5).getString();
            event.d_payload     = nlohmann::json::parse(payload);
            event.d_timestamp   = query.getColumn(6).getInt64();

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
    int64_t                       start_ts,
    const std::optional<int64_t>& end_ts) noexcept(true)
{
    // pass
    SQLite::Statement query(d_db,
                            "SELECT id, symbol, qty, price, type, payload, "
                            "timestamp FROM events WHERE timestamp >= ? AND "
                            "timestamp <= ? AND symbol = ?");

    Events events;
    try {
        query.bind(1, start_ts);
        query.bind(2, end_ts.value_or(common::getCurrentTimestampMs()));
        query.bind(3, symbol);

        while (query.executeStep()) {
            Event event;
            event.d_symbol = query.getColumn(1).getString();
            event.d_qty    = query.getColumn(2).getDouble();
            event.d_price  = query.getColumn(3).getDouble();
            event.d_type = static_cast<EventType>(query.getColumn(4).getInt());
            std::string payload = query.getColumn(5).getString();
            event.d_payload     = nlohmann::json::parse(payload);
            event.d_timestamp   = query.getColumn(6).getInt64();

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
            d_db.exec(CREATE_EVENTS_TABLE);
            d_db.exec(CREATE_INDEX_SYMBOL_TS);
            d_db.exec(CREATE_INDEX_TYPE_TS);
            d_db.exec(CREATE_INDEX_SYMBOL_TYPE_TS);
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
    std::vector<Event>   batch;
    batch.reserve(BATCH_SIZE);

    Event event;
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

bool MarketEventsDb::logEventSync(const Event& event)
{
    return logEvent({event});
}

bool MarketEventsDb::logEventAsync(const Event& event)
{
    if (!d_eventQueue.push(event)) {
        SPDLOG_WARN("Event queue full, dropping event...");
        return false;
    }
    return true;
}

} // namespace databases
} // namespace crypto_trader
