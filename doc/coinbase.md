# Overview
There are two types of coinbase APIs:  
[Documentation](https://docs.cloud.coinbase.com/exchange/docs/welcome)
- Trading APIs  
    - Trading API's require authentication and rely on some account being attached.  
    - There is a low and high frequency variant (depending on the strategy we may want one or the other?)
- Market Data APIs  
    - Are public and provide market data  
    - We should start here before placing trades  

## Market Data API:
There are two types  
There are a production and sandbox versions of both  
1. Exchange WebSocket  
- Seems like an API key may not be necessary for this one
- Provides real-time market data updates for orders and trades  
- In JSON format
- Subscribe to the endpoint:
```
Coinbase Market Data
production = wss://ws-feed.exchange.coinbase.com
sandbox = wss://ws-feed-public.sandbox.exchange.coinbase.com


Coinbase Direct Market Data
production = wss://ws-direct.exchange.coinbase.com
sandbox = wss://ws-direct.sandbox.exchange.coinbase.com
```

- A "type" attribute tells what type of message is, and unknown types or new types can be expected, and should be ignored  
- If a "subscribe" message is not recieved by the server within 5 seconds, you are automatically unsubscribed  
- Messages are sent with a sequence number, but messages are not guaranteed to be in order, so logic is necessary to interpret both dropped messages and messages recieved out of order.  
- There are different types of channels that can be subscribed to, see [this](https://docs.cloud.coinbase.com/exchange/docs/websocket-channels)  


How can we connect to a websocket in C++?
- [WebSocket++](https://docs.websocketpp.org/)
- [Boost ASIO](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio.html) and [Boost Beast](https://www.boost.org/doc/libs/1_80_0/libs/beast/doc/html/index.html) (I prefer this one, although it's probably harder to learn it will be more robust)

2. Exchange "FIX"  
- a "FIX" API (no idea what that means, spec [here](https://www.onixs.biz/fix-dictionary/5.0.sp2/index.html))  
- One connection per API key  
