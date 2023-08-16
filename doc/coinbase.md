# Overview
There are two types of coinbase APIs:  
[Documentation](https://docs.cloud.coinbase.com/exchange/docs/welcome)  
- Trading APIs  
    - Trading API's require authentication and rely on some account being
      attached.  
    - There is a low and high frequency variant (depending on the strategy we
      may want one or the other?)
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

- A "type" attribute tells what type of message is, and unknown types or new
  types can be expected, and should be ignored  
- If a "subscribe" message is not recieved by the server within 5 seconds, you
  are automatically unsubscribed  
- Messages are sent with a sequence number, but messages are not guaranteed to
  be in order, so logic is necessary to interpret both dropped messages and
  messages recieved out of order.  
- There are different types of channels that can be subscribed to, see
  [this](https://docs.cloud.coinbase.com/exchange/docs/websocket-channels)  


How can we connect to a websocket in C++?
- [WebSocket++](https://docs.websocketpp.org/)
- [Boost ASIO](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio.html)
  and [Boost
  Beast](https://www.boost.org/doc/libs/1_80_0/libs/beast/doc/html/index.html)
  (I prefer this one, although it's probably harder to learn it will be more
  robust)


2. Market Data [FIX](https://docs.cloud.coinbase.com/exchange/docs/fix-msg-market-data)  
- They describe it as "The FIX market data API provides an L3 feed only with direct, low-latency, deterministic access."  
    - Question: What is "L3"? a type of market data?
- One connection per API key  
- FIX = Financial Information Exchange, it is an finance specific protocol for different types of financial operations  
- for market data, they use a slightly extended [Version 5.0](https://www.onixs.biz/fix-dictionary/5.0.sp2/index.html)    
```
ENVIRONMENT URLS
Production: tcp+ssl://fix-md.exchange.coinbase.com:6121
Sandbox: tcp+ssl://fix-md.sandbox.exchange.coinbase.com:6121
```
Types of messages:   
    - Logon  
    - Logout   
    - Market Data Request (This is to subscribe to a symbol)  
    - Security Satus   
        - This is the message recieved from the market for the subscription  
    - Market Data Incremental Refresh  
        - Looks like a way to get the order book in real time (i.e open offers to buy and sell & what prices)  
        - Also indicates if the orders are our own  
    - Market Data Snapshot Full Refresh
        - A full view of the order book.
        - This is done automatically whenever there is a market data request for a given symbol
    - Security List Request
        - Gets a Security List, all of the "instruments" (Coins & other things like limit trades? At least coins)

Trading APIs:

1. [REST Exchange API](https://docs.cloud.coinbase.com/exchange/docs/rest-requests)
- [Full reference](https://docs.cloud.coinbase.com/exchange/reference)  
- Low frequency trades  
- You can get a private endpoint by using an API key  
- In order to trade with the API key must give it proper permissions,   
    - Permissions are "View", "Transfer" (can withdraw and deposit), "Trade" (can make orders)  
- You are rate limited to 10  requests per second on a public endpoint, 15 on a private one  

2. FIX Order Entry [FIX](https://docs.cloud.coinbase.com/exchange/docs/fix-msg-order-entry)  
- One connection per API key, to have more, generate more keys  
- FIX = Financial Information Exchange, it is an finance specific protocol for different types of financial operations  
- for order entries, they use a slightly extended [Version 4.2](https://www.onixs.biz/fix-dictionary/4.2/index.html)    
```
ENVIRONMENT URLS
Production: tcp+ssl://fix.exchange.coinbase.com:4198
Sandbox: tcp+ssl://fix-public.sandbox.exchange.coinbase.com:4198
```
Types of messages:  
    - Logon  
    - Logout (This and logon establish a session)  
    - New Order Single  
    - New Order Batch  
    - Order Cancel Request  
    - Order Status Request  
    - Order Cancel Batch Request  
    - Modify Order Request    
    - Execution report (this is basically the response type for each message)   
    - Quote Request (it is unclear to me whether this is for a liquidity provider or for us. I'm leaning towards liquiudity provider)   
    - Heartbeat (The time for a heartbeat is set during logon).  
