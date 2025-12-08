#ifndef INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H
#define INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H

#include "Event.h"
#include <list>
#include <map>
#include <vector>

struct Position {
    double  d_total_qty = 0.0;
    double  d_price     = 0.0;
    int64_t d_timestamp;
};

struct SymbolPositions {
    std::string         d_symbol;
    double              d_total_qty         = 0.0;
    double              d_average_price     = 0.0;
    std::list<Position> d_positions_in_time = {};
    bool                d_fifo              = true;
    double              d_realizedPnl       = 0.0;
};

class Accounting {
  public:
    // TYPES
    using PositionMap = std::map<std::string, SymbolPositions>;

    void               apply_event(const Event& e);
    const PositionMap& snapshot() const;
    void               replay_events(const std::vector<Event>& events);

    // CREATORS
    Accounting() = default;

  private:
    std::vector<Event> d_event_log_;
    PositionMap        d_positions_;
};

#endif // INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H
