#ifndef INCLUDED_DATABASE_WEBSOCKET_CLIENT
#define INCLUDED_DATABASE_WEBSOCKET_CLIENT

#include "../databases/market_data_db.h"
#include "../protocols/websocket_client.h"

#include <memory>

namespace crypto_trader {
namespace adaptors {

template <typename MarketDataType>
class DatabaseWebsocketClient : public protocols::WebsocketClient {
  public:
    // PUBLIC TYPES
    using MarketDataDb    = databases::MarketDataDB<MarketDataType>;
    using Timestamp       = typename MarketDataType::Timestamp;
    using MarketDataDbPtr = std::shared_ptr<MarketDataDb>;

  private:
    // PRIVATE DATA
    MarketDataDbPtr d_db_p;

  public:
    // CREATORS
    DatabaseWebsocketClient(const MarketDataDbPtr& dbPtr);
    ~DatabaseWebsocketClient();

    // PUBLIC MANIPULATORS
    void start();
    void stop();

    // protocols::WebsocketClient
    bool open() override;
    void close() override;
    bool is_open() override;
    bool send_message() override;
    void listen() override;

}; // class DatabaseWebsocketClient

// class DatabaseWebsocketClient

// CREATORS
template <typename MarketDataType>
DatabaseWebsocketClient<MarketDataType>::DatabaseWebsocketClient(
    const MarketDataDbPtr& dbPtr)
: d_db_p(dbPtr)
{
}

template <typename MarketDataType>
DatabaseWebsocketClient<MarketDataType>::~DatabaseWebsocketClient()
{
}

// PUBLIC MANIPULATORS
template <typename MarketDataType>
void DatabaseWebsocketClient<MarketDataType>::start()
{
}

template <typename MarketDataType>
void DatabaseWebsocketClient<MarketDataType>::stop()
{
}

// protocols::WebsocketClient

template <typename MarketDataType>
bool DatabaseWebsocketClient<MarketDataType>::open()
{
    return true;
}

template <typename MarketDataType>
void DatabaseWebsocketClient<MarketDataType>::close()
{
}

template <typename MarketDataType>
bool DatabaseWebsocketClient<MarketDataType>::is_open()
{
    return true;
}

template <typename MarketDataType>
bool DatabaseWebsocketClient<MarketDataType>::send_message()
{
    return true;
}

template <typename MarketDataType>
void DatabaseWebsocketClient<MarketDataType>::listen()
{
}

} // namespace adaptors
} // namespace crypto_trader

#endif // INCLUDED_DATABASE_WEBSOCKET_CLIENT
