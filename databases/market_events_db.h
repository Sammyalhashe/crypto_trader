#ifndef INCLUDED_MARKET_EVENTS_DB
#define INCLUDED_MARKET_EVENTS_DB

#include "../common/Event.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <memory>

namespace crypto_trader {
namespace databases {

class MarketEventsDb {

  private:
    // PRIVATE DATA
    SQLite::Database d_db;

  public:
    // TYPES
    using MarketEventsDbPtr = std::shared_ptr<MarketEventsDb>;

    // CREATORS
    explicit MarketEventsDb(const std::string& dbPath);

    // PUBLIC MANIPULATORS
    bool init() noexcept(true);
    bool logEvent(const Event& event);

  private:
    // PRIVATE MANIPULATORS
    bool setupPerformance() noexcept(true);
    bool runMigrations() noexcept(true);

}; // class MarketEventsDb

}; // namespace databases
}; // namespace crypto_trader

#endif // INCLUDED_MARKET_EVENTS_DB
