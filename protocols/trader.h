#ifndef INCLUDED_TRADER
#define INCLUDED_TRADER

#include <string_view>

namespace crypto_trader {
namespace protocols {

/**
 * @brief Abstract base class defining the interface for a trading entity
 * (Trader).
 *
 * A Trader orchestrates the interaction between market data sources (e.g.,
 * WebSockets), trading strategies, and execution mechanisms.
 */
class Trader {

  public:
    /**
     * @brief Constructs a Trader object.
     */
    Trader();

    /**
     * @brief Destructor for the Trader interface.
     */
    virtual ~Trader() = 0;

    /**
     * @brief Processes incoming raw market data or messages from an external
     * source.
     * @param buffer The raw data buffer containing market information.
     */
    virtual void listen(const std::string_view& buffer) = 0;

    /**
     * @brief Starts the trading process, including connecting to data sources
     * and initializing strategies/executors.
     */
    virtual void start() = 0;

    /**
     * @brief Stops the trading process, disconnecting from data sources and
     *        performing necessary cleanup.
     */
    virtual void stop() = 0;

}; // Trader

} // namespace protocols
} // namespace crypto_trader
#endif // INCLUDED_TRADER