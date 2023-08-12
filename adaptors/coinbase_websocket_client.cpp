#include "coinbase_websocket_client.h"

namespace crypto_trader {
namespace adaptors {

// CoinbaseWebSocketClient

// CREATORS
CoinbaseWebSocketClient::CoinbaseWebSocketClient()
{}

CoinbaseWebSocketClient::~CoinbaseWebSocketClient()
{}


// MANIPULATORS

// PRIVATE MANIPULATORS

// WebsocketClient
bool CoinbaseWebSocketClient::open()
{
    return true;
}

void CoinbaseWebSocketClient::close()
{

}

bool CoinbaseWebSocketClient::is_open()
{
    return true;
}

bool CoinbaseWebSocketClient::send_message()
{
    return true;
}

} // adaptors
} // crypto_trader
