#ifndef INCLUDED_MARKET_EVENTS_DB
#define INCLUDED_MARKET_EVENTS_DB

#include "../common/Accounting.h"
#include "../common/Event.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <boost/lockfree/spsc_queue.hpp>

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace crypto_trader {
namespace databases {

class MarketEventsDb {

  private:
    // PRIVATE DATA
    // sqlite database
    SQLite::Database d_db;
    // queue for batching writes to database
    boost::lockfree::spsc_queue<common::Event, boost::lockfree::capacity<10000>>
        d_eventQueue;
    // thread for writing the batched events to the db
    std::jthread      d_writerThread;
    std::atomic<bool> d_stopWriter{false};

  public:
    // TYPES
    using MarketEventsDbPtr  = std::shared_ptr<MarketEventsDb>;
    using Events             = std::vector<common::Event>;
    using SymbolPositionsVec = std::vector<common::SymbolPositions>;

    // CREATORS
    explicit MarketEventsDb(const std::string& dbPath);
    ~MarketEventsDb();

    // PUBLIC MANIPULATORS
    /**
     * @brief Initialized the database by setting up performance
     * configurations, running migrations, and starting up an async thread for
     * batch writes
     * @return `bool` if all succeeded
     */
    bool init() noexcept(true);
    /**
     * @brief log a single event to the `events` table.
     * Smartly routes events based on their type to either be logged
     * synchronous or async This method does not throw.
     * @return `bool` if successful
     */
    bool logEvent(const common::Event& event);
    /**
     * @brief log a batch of events to the `events` table.
     * @return `bool` if successful
     */
    bool logEvents(const std::vector<common::Event>& events);
    /**
     * @brief Return a vector containing the `Event`s since the given
     * `timestamp`. This method does not throw, but returns `std::nullopt` if
     * something goes wrong.
     * @return `std::optional<std::vector<Event>>`
     */
    std::optional<Events> getEventsSince(int64_t timestamp) noexcept(true);
    /**
     * @brief Return a vector containing the `Event`s for a given `symbol`
     * between the given `start_ts` and `end_ts`. If the optional `end_ts`
     * isn't given, assume `end_ts` = present. This method does not throw, but
     * returns `std::nullopt` if something goes wrong.
     * @return `std::optional<std::vector<Event>>`
     */
    std::optional<Events> getEventsBySymbol(
        const std::string&            symbol,
        int64_t                       start_ts,
        const std::optional<int64_t>& end_ts = std::nullopt) noexcept(true);

    /**
     * @brief Write a single snapshot synchronously
     * @param snapshot
     * @return `bool` on success
     */
    bool logSnapshot(const common::SymbolPositions& snapshot) noexcept(true);

    /**
     * @brief Write a group of snapshots synchronously in a batch
     * @param snapshots
     * @return `bool` on success
     */
    bool
    logSnapshots(const std::vector<common::SymbolPositions>& snapshots) noexcept(
        true);

    /**
     * @brief get latest snapshot for a given `symbol`
     * @param symbol
     * @return `std::optional<common::SymbolPositions>`
     */
    std::optional<common::SymbolPositions>
    getLatestSnapshot(const std::string& symbol) noexcept(true);

    /**
     * @brief get latest snapshot for all symbols
     * @return `std::optional<common::SymbolPositions>`
     */
    std::optional<SymbolPositionsVec> getLatestSnapshots() noexcept(true);

  private:
    // PRIVATE MANIPULATORS
    bool setupWriterThread();
    bool setupPerformance() noexcept(true);
    bool runMigrations() noexcept(true);
    void writerThreadLoop() noexcept(true);
    bool logEventSync(const common::Event& event);
    bool logEventAsync(const common::Event& event);

}; // class MarketEventsDb

}; // namespace databases
}; // namespace crypto_trader

#endif // INCLUDED_MARKET_EVENTS_DB
