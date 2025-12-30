#ifndef INCLUDED_COINBASE_TRADER
#define INCLUDED_COINBASE_TRADER

#include "../common/types.h"
#include "../databases/market_data_db.h"
#include "../databases/market_events_db.h"
#include "../protocols/executor.h"
#include "../protocols/strategy.h"
#include "../protocols/trader.h"
#include "../protocols/websocket_client.h"
#include "../strategies/index.h"
#include "event_position_manager.h"

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/optional.hpp>

#include <nlohmann/json.hpp>

#include <atomic>
#include <memory>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>

namespace crypto_trader {
namespace traders {

class CoinbaseTraderConfig {
  public:
    // PUBLIC TYPES
    struct ChannelDefinition {
        std::string              d_name;
        std::vector<std::string> d_products;
    }; // ChannelDefinition

    enum class ClientType { SYNC, ASYNC, COUNT }; // ClientType

  private:
    // PRIVATE TYPES
    using StringVec  = std::vector<std::string>;
    using Channel    = std::variant<std::string, ChannelDefinition>;
    using ChannelVec = std::vector<Channel>;

    // PRIVATE DATA
    // The products to monitor/trade.
    StringVec d_products;
    // The channels to subscribe to if using a websocket strategy
    boost::optional<ChannelVec> d_channels;
    // The strategy to use to trade with.
    strategies::TradingStrategy d_strategy;
    // The url the trader gets the data from.
    std::string d_url;
    // abstract json config for the strategy.
    nlohmann::json d_strategyConfig;
    // The number of threads the trader's threadpool will run.
    unsigned int d_numThreads;
    // use the async client?
    ClientType d_clientType;
    // Shared atomic state declaring whether the application is running.
    std::shared_ptr<std::atomic_bool> d_isRunning;
    // Determines whether to use the paper or real trading executor.
    bool                                         d_paperTrading;
    databases::MarketEventsDb::MarketEventsDbPtr d_db_p;

  public:
    // CREATORS
    explicit CoinbaseTraderConfig(
        const std::shared_ptr<std::atomic_bool>& isRunning,
        bool                                     paperTrading = false);
    CoinbaseTraderConfig(const CoinbaseTraderConfig& orig) = default;

    // PUBLIC MANIPULATORS
    CoinbaseTraderConfig& setProducts(const StringVec& products);
    CoinbaseTraderConfig&
    setChannels(const boost::optional<ChannelVec>& products);
    CoinbaseTraderConfig& setChannels(const ChannelVec& products);
    CoinbaseTraderConfig&
    setStrategy(const strategies::TradingStrategy& strategy);
    CoinbaseTraderConfig& setUrl(const std::string& url);
    CoinbaseTraderConfig&
    setStrategyConfig(const nlohmann::json& strategyConfig);
    CoinbaseTraderConfig& setNumThreads(unsigned int numThreads);
    CoinbaseTraderConfig& setClientType(const ClientType clientType);
    CoinbaseTraderConfig& setPaperTrading(bool paperTrading);
    CoinbaseTraderConfig&
    setEventsDb(const databases::MarketEventsDb::MarketEventsDbPtr& db);

    // PUBLIC ACCESSORS
    const StringVec&                                    products() const;
    const boost::optional<ChannelVec>&                  channels() const;
    const strategies::TradingStrategy&                  strategy() const;
    const std::string&                                  url() const;
    const nlohmann::json&                               strategyConfig() const;
    unsigned int                                        numThreads() const;
    const ClientType                                    clientType() const;
    const std::shared_ptr<std::atomic_bool>&            isRunning() const;
    bool                                                paperTrading() const;
    const databases::MarketEventsDb::MarketEventsDbPtr& eventsDb() const;
}; // CoinbaseTraderConfig

class CoinbaseTrader : public protocols::Trader {
  private:
    // STATIC DATA
    // The file that the database loads/saves data to.
    static const char *s_databaseFile;
    // PRIVATE DATA
    // Websocket client that may or may not be used.
    std::shared_ptr<protocols::WebsocketClient> d_webSocketClient;
    // The strategy this trader has decided to use.
    std::unique_ptr<protocols::Strategy> d_strategy;
    // boost thread to offload items from websocket client
    boost::asio::io_context d_ioCtx;
    std::jthread            d_thread;
    // this keeps the number of processing tasks for the iocontext artificially
    // at zero to make sure it blocks
    // should be released on stop.
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        d_ioWorkGuard;
    // The database that stores the data received from clients and other
    // trade data.
    databases::MarketDataDB<common::MarketDataCoinbase> d_database;
    // If this trader is running or not.
    std::atomic_bool d_isStopped;
    // The config for this trader.
    CoinbaseTraderConfig d_config;
    // The executor to execute trades on.
    std::unique_ptr<protocols::Executor<common::MarketDataCoinbase>>
        d_executor;
    // sequence numbers received for each product
    std::unordered_map<std::string, int64_t> d_lastSequenceNumbers;
    // Position Manager
    EventPositionManager d_positionManager;

  public:
    // CREATORS
    CoinbaseTrader(const CoinbaseTraderConfig& config);
    ~CoinbaseTrader();

    // DELETED METHODS
    CoinbaseTrader(const CoinbaseTrader& orig)            = delete;
    CoinbaseTrader& operator=(const CoinbaseTrader& orig) = delete;

    // PUBLIC MANIPULATORS
    // Process an incoming action.
    void processAction(const common::Action& action);
    // Handle new data available to the trader.
    void handleNewData(const std::string_view& buffer);

    // protocols::Trader
    void listen(const std::string_view& buffer) override;
    // Start the trader.
    void start() override;
    // Stop the trader.
    void stop() override;

  private:
    // PRIVATE MANIPULATORS
    void initWebsocketClient();

    bool checkSequenceNumber(const std::string_view& product,
                             int64_t                 sequence);
    void handleTickerMessage(const nlohmann::json& msg);

}; // CoinbaseTrader

// INLINE DEFINITIONS
// class CoinbaseTraderConfig

// TYPES
// ClientType

inline std::ostream&
operator<<(std::ostream&                           out,
           const CoinbaseTraderConfig::ClientType& clientType)
{
    switch (clientType) {
    case CoinbaseTraderConfig::ClientType::SYNC: {
        out << "SYNC";
    } break;
    case CoinbaseTraderConfig::ClientType::ASYNC: {
        out << "ASYNC";
    } break;
    case CoinbaseTraderConfig::ClientType::COUNT: {
        out << "COUNT";
    } break;
    default: {
        out << "UNKNOWN";
    }
    }
    return out;
}

// PUBLIC MANIPULATORS
inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setProducts(const std::vector<std::string>& products)
{
    d_products = products;
    return *this;
}

inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setChannels(const boost::optional<ChannelVec>& channels)
{
    d_channels = channels;
    return *this;
}

inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setChannels(const ChannelVec& products)
{
    d_channels = products;
    return *this;
}

inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setStrategy(const strategies::TradingStrategy& strategy)
{
    d_strategy = strategy;
    return *this;
}

inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setUrl(const std::string& url)
{
    d_url = url;
    return *this;
}

inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setStrategyConfig(const nlohmann::json& strategyConfig)
{
    d_strategyConfig = strategyConfig;
    return *this;
}

inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setNumThreads(unsigned int numThreads)
{
    d_numThreads = numThreads;
    return *this;
}

inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setClientType(const ClientType clientType)
{
    d_clientType = clientType;
    return *this;
}

inline CoinbaseTraderConfig&
CoinbaseTraderConfig::setPaperTrading(bool paperTrading)
{
    d_paperTrading = paperTrading;
    return *this;
}

inline CoinbaseTraderConfig& CoinbaseTraderConfig::setEventsDb(
    const databases::MarketEventsDb::MarketEventsDbPtr& db)
{
    d_db_p = db;
    return *this;
}

// PUBLIC ACCESSORS
inline const CoinbaseTraderConfig::StringVec&
CoinbaseTraderConfig::products() const
{
    return d_products;
}

inline const boost::optional<CoinbaseTraderConfig::ChannelVec>&
CoinbaseTraderConfig::channels() const
{
    return d_channels;
}

inline const strategies::TradingStrategy&
CoinbaseTraderConfig::strategy() const
{
    return d_strategy;
}

inline const std::string& CoinbaseTraderConfig::url() const { return d_url; }

inline const nlohmann::json& CoinbaseTraderConfig::strategyConfig() const
{
    return d_strategyConfig;
}

inline unsigned int CoinbaseTraderConfig::numThreads() const
{
    return d_numThreads;
}

inline const CoinbaseTraderConfig::ClientType
CoinbaseTraderConfig::clientType() const
{
    return d_clientType;
}

inline const std::shared_ptr<std::atomic_bool>&
CoinbaseTraderConfig::isRunning() const
{
    return d_isRunning;
}

inline bool CoinbaseTraderConfig::paperTrading() const
{
    return d_paperTrading;
}

inline const databases::MarketEventsDb::MarketEventsDbPtr&
CoinbaseTraderConfig::eventsDb() const
{
    return d_db_p;
}

} // namespace traders
} // namespace crypto_trader
#endif // INCLUDED_COINBASE_TRADER
