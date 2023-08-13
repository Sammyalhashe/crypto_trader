#include "adaptors/coinbase_websocket_client.h"
#include "strategies/simple.h"
#include "testlib/test.h"

#include <cstdlib>
#include <cstring>
#include <stdio.h>

int main() {
    using namespace crypto_trader;
    strategies::simpleStrategy();

    adaptors::CoinbaseWebSocketClient client;

    return 0;
}
