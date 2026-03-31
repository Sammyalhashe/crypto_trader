# TODO

## Phase 1: Paper Trading Foundation

- [ ] **Realistic paper executor** — Track entry prices per symbol, calculate P&L on sells, simulate round-trip fees (0.3% for Coinbase Advanced 3 tier)
- [ ] **State persistence** — Save/load positions, entry prices, peak portfolio value, and trade history to JSON or SQLite between runs. Losing context on restart makes testing meaningless
- [ ] **Position sizing** — Don't buy a fixed $100 every time. Scale position size relative to portfolio value and number of open positions
- [ ] **Basic risk controls** — Max concurrent positions (e.g. 3), minimum order size, stop-loss per trade

## Phase 2: Technical Indicators & Strategy

- [ ] **EMA (Exponential Moving Average)** — Implement 50/200 EMA for trend detection. This is the foundation for golden cross / death cross signals
- [ ] **RSI (Relative Strength Index)** — Filter out overbought entries (>75). Prevents buying into pumps
- [ ] **ATR (Average True Range)** — Use for dynamic trailing stop distances instead of fixed percentages
- [ ] **Replace HODL strategy** — Implement trend-following with MA crossover + RSI filter. The current 0.1% thresholds trigger constantly and get eaten alive by fees
- [ ] **Market regime detection** — Use BTC's MA trend to classify BULL/NEUTRAL/BEAR. Skip entries in bear markets

## Phase 3: Backtesting

- [ ] **CSV data loader** — Read OHLCV candle data (timestamp, open, high, low, close, volume). Coinbase public API provides this without authentication
- [ ] **Backtest engine** — Iterate candles chronologically, feed them through the strategy, simulate paper trades with fee deduction
- [ ] **Performance metrics** — Calculate total return, max drawdown, Sharpe ratio, win rate, profit factor
- [ ] **Historical data downloader** — Pull candle data from Coinbase public API for BTC, ETH, SOL across different timeframes (1h, 1d)

## Phase 4: Exchange Integration

- [ ] **Coinbase REST API client** — JWT signing for authenticated requests (account balances, place orders). Boost.Beast already handles HTTP
- [ ] **Order management** — Market orders first, then limit orders (maker fee is 0.075% vs 0.15% taker — worth the complexity)
- [ ] **Error handling & retries** — Network failures, rate limits, partial fills

## Phase 5: C++ Differentiators

- [ ] **Real-time order book** — Use the existing Zig order book module with L2 WebSocket feed. Detect liquidity walls, spreads, imbalances
- [ ] **Tick-level backtesting** — Process millions of ticks efficiently. This is where C++ justifies itself over Python
- [ ] **Smart order placement** — TWAP/VWAP execution to minimize slippage on larger orders
