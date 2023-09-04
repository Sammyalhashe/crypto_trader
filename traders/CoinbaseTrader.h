#ifndef INCLUDED_COINBASE_TRADER
#define INCLUDED_COINBASE_TRADER

#include "../protocols/websocket_client.h"
#include "../protocols/strategy.h"
#include "../protocols/trader.h"
#include "../strategies/index.h"

#include <boost/optional.hpp>

#include <atomic>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace crypto_trader {
namespace traders {

class CoinbaseTraderConfig {
public:
    // PUBLIC TYPES
    struct ChannelDefinition {
        std::string d_name;
        std::vector<std::string> d_products;
    }; // ChannelDefinition
private:
    // PRIVATE TYPES
    using StringVec = std::vector<std::string>;
    using Channel = std::variant<std::string, ChannelDefinition>;
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
    // Shared atomic state declaring whether the application is running.
    std::shared_ptr<std::atomic_bool> d_isRunning;


public:
    // CREATORS
    explicit CoinbaseTraderConfig(
                           const std::shared_ptr<std::atomic_bool>& isRunning);
    CoinbaseTraderConfig(const CoinbaseTraderConfig& orig) = default;

    // PUBLIC MANIPULATORS
    CoinbaseTraderConfig& setProducts(const StringVec& products);
    CoinbaseTraderConfig& setChannels(
                                  const boost::optional<ChannelVec>& products);
    CoinbaseTraderConfig& setChannels(const ChannelVec& products);
    CoinbaseTraderConfig& setStrategy(
                                  const strategies::TradingStrategy& strategy);
    CoinbaseTraderConfig& setUrl(const std::string& url);

    // PUBLIC ACCESSORS
    const StringVec& products() const;
    const boost::optional<ChannelVec>& channels() const;
    const strategies::TradingStrategy& strategy() const;
    const std::string& url() const;
    const std::shared_ptr<std::atomic_bool>& isRunning() const;
}; // CoinbaseTraderConfig

class CoinbaseTrader : public protocols::Trader {
private:
    // PRIVATE DATA
    // Websocket client that may or may not be used.
    std::unique_ptr<protocols::WebsocketClient> d_webSocketClient;
    // The strategy this trader has decided to use.
    std::unique_ptr<protocols::Strategy> d_strategy;
    // The config for this trader.
    CoinbaseTraderConfig d_config;

public:
    // CREATORS
    CoinbaseTrader(const CoinbaseTraderConfig& config);
    ~CoinbaseTrader();

    // DELETED METHODS
    CoinbaseTrader(const CoinbaseTrader& orig) = delete;
    CoinbaseTrader& operator=(const CoinbaseTrader& orig) = delete;

    // PUBLIC MANIPULATORS
    
    // Start the trader.
    void start();
    // Stop the trader.
    void stop();

    // protocols::Trader
    void listen(const std::string_view& buffer) override;

}; // CoinbaseTrader

// INLINE DEFINITIONS
// class CoinbaseTraderConfig

// PUBLIC MANIPULATORS
inline
CoinbaseTraderConfig& CoinbaseTraderConfig::setProducts(
                                      const std::vector<std::string> &products)
{
    d_products = products;
    return *this;
}

inline
CoinbaseTraderConfig& CoinbaseTraderConfig::setChannels(
                const boost::optional<ChannelVec> &channels)
{
    d_channels = channels;
    return *this;
}

inline
CoinbaseTraderConfig& CoinbaseTraderConfig::setChannels(
                                                    const ChannelVec& products)
{
    d_channels = products;
    return *this;
}

inline
CoinbaseTraderConfig& CoinbaseTraderConfig::setStrategy(
                                          const strategies::TradingStrategy &strategy)
{
    d_strategy = strategy;
    return *this;
}

inline
CoinbaseTraderConfig& CoinbaseTraderConfig::setUrl(const std::string& url)
{
    d_url = url;
    return *this;
}

// PUBLIC ACCESSORS
inline
const CoinbaseTraderConfig::StringVec& CoinbaseTraderConfig::products() const
{
    return d_products;
}

inline
const boost::optional<CoinbaseTraderConfig::ChannelVec>&
CoinbaseTraderConfig::channels() const
{
    return d_channels;
}

inline
const strategies::TradingStrategy& CoinbaseTraderConfig::strategy() const
{
    return d_strategy;
}

inline
const std::string& CoinbaseTraderConfig::url() const
{
    return d_url;
}

inline
const std::shared_ptr<std::atomic_bool>& CoinbaseTraderConfig::isRunning() const
{
    return d_isRunning;
}

} // traders
} // crypto_trader
#endif // INCLUDED_COINBASE_TRADER
