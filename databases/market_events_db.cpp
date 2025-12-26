#include "market_events_db.h"

#include "../common/Event.h"

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Exception.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>

#include <spdlog/spdlog.h>
#include <string>

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

// MarketEventsDb

// CREATORS
MarketEventsDb::MarketEventsDb(const std::string& dbPath)
: d_db(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
}

// PUBLIC MANIPULATORS
bool MarketEventsDb::init() noexcept(true)
{
    return setupPerformance() && runMigrations();
}

bool MarketEventsDb::logEvent(const Event& event)
{
    // prepare the statement
    SQLite::Statement query(
        d_db,
        "INSERT INTO events (symbol, qty, price, type, payload, timestamp)"
        "VALUES (?, ?, ?, ?, ?, ?)");

    // bind the values
    query.bind(1, event.d_symbol);
    query.bind(2, event.d_qty);
    query.bind(3, event.d_price);
    query.bind(4, static_cast<int>(event.d_type));
    query.bind(5, event.d_payload.dump());
    query.bind(6, event.d_timestamp);

    // execute
    try {
        query.exec();
    }
    catch (const SQLite::Exception& e) {
        SPDLOG_ERROR("Unable to execute query {} with error {}",
                     query.getExpandedSQL(),
                     e.getErrorStr());
        return false;
    }
    return true;
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

} // namespace databases
} // namespace crypto_trader
