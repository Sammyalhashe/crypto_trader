#ifndef INCLUDED_TRADER
#define INCLUDED_TRADER

#include <string_view>

namespace crypto_trader {
namespace protocols {

class Trader {

  protected:
      // PROTECTED DATA
      bool d_trade;  // Should the trader actually perform a trade?

      // PROTECTED MANIPULATORS
      void setTrade(bool value);

  public:
    // CREATORS
    Trader();
    virtual ~Trader() = 0;

    // PUBLIC ACCESSORS
    virtual std::string name() = 0;

    // PUBLIC MANIPULATORS
    virtual void listen(const std::string_view& buffer) = 0;
    virtual void start()                                = 0;
    virtual void stop()                                 = 0;

}; // Trader

inline
void Trader::setTrade(bool value) {
    d_trade = value;
}

} // namespace protocols
} // namespace crypto_trader
#endif // INCLUDED_TRADER
