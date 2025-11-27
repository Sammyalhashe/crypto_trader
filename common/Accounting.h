#ifndef INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H
#define INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H

#include "Event.h"
#include <map>
#include <vector>

struct Position {
    double d_total_qty     = 0.0;
    double d_average_price = 0.0;
};

class Accounting {
  public:
    void                                   apply_event(const Event& e);
    const std::map<std::string, Position>& snapshot() const;
    void replay_events(const std::vector<Event>& events);

    // CREATORS
    Accounting() = default;

  private:
    std::vector<Event>              d_event_log_;
    std::map<std::string, Position> d_positions_;
};

#endif // INCLUDED_CRYPTO_TRADER_COMMON_ACCOUNTING_H
