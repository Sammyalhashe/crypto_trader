#ifndef INCLUDED_MARKET_DATA_DB
#define INCLUDED_MARKET_DATA_DB

#include "../common/serialization.h"
#include "../common/types.h"

#include <fstream>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace crypto_trader {
namespace databases {

// TODO: add multithread support (mutexes)
// Read / Write lock per symbol?
// Read / Write lock for the entire map
// Potentially set up each symbol beforehand to not lock the map.
// If locking map may as well have 1 lock
template <common::MarketData MarketDataType>
class MarketDataDB {
  private:
    // PRIVATE DATA
    std::unordered_map<std::string, std::vector<MarketDataType>>
        d_symbol_to_data;

  public:
    // CREATORS
    MarketDataDB() {}
    ~MarketDataDB() {}

    // MANIPULATORS
    inline void add_data(const std::string&    symbol,
                         const MarketDataType& market_data) noexcept;

    bool load(std::string& file_name);

    // ACCESSORS
    std::vector<MarketDataType>
    get_data(const std::string&                        symbol,
             const typename MarketDataType::Timestamp& min_ts,
             const typename MarketDataType::Timestamp& max_ts) const;

    // TODO: make an error status type for saving and loading.
    bool save(const std::string& file_name) const;

  private:
    // PRIVATE MANIPULATORS
    template <class Archive>
    void serialize(Archive& archive, const unsigned int version);

    // FRIENDS
    friend class boost::serialization::access;
}; // MarketDataDB

// class MarketDataDB

// PUBLIC MANIPULATORS
template <common::MarketData MarketDataType>
inline void MarketDataDB<MarketDataType>::add_data(
    const std::string& symbol, const MarketDataType& market_data) noexcept
{
    // TODO: ADD a happy path:
    // check if the last element is at a earlier time than the new dat with the
    // ordering predicate.
    if (!d_symbol_to_data.contains(symbol)) {
        d_symbol_to_data[symbol] = std::vector<MarketDataType>();
    }

    auto& data_vec = d_symbol_to_data[symbol];
    data_vec.insert(std::upper_bound(data_vec.begin(),
                                     data_vec.end(),
                                     market_data,
                                     MarketDataType::order),
                    market_data);
}

template <common::MarketData MarketDataType>
bool MarketDataDB<MarketDataType>::load(std::string& file_name)
{
    std::ifstream in_stream(file_name, std::ofstream::binary);
    boost::archive::binary_iarchive in_arch(in_stream);
    in_arch&*(this);
    return true;
}

// PUBLIC ACCESSORS
template <common::MarketData MarketDataType>
std::vector<MarketDataType> MarketDataDB<MarketDataType>::get_data(
    const std::string&                        symbol,
    const typename MarketDataType::Timestamp& min_ts,
    const typename MarketDataType::Timestamp& max_ts) const
{

    if (!d_symbol_to_data.contains(symbol))
        return {};

    auto& data_vec = d_symbol_to_data.at(symbol);
    return std::vector<MarketDataType>(
        std::lower_bound(
            data_vec.begin(), data_vec.end(), min_ts, MarketDataType::order),
        std::upper_bound(
            data_vec.begin(), data_vec.end(), max_ts, MarketDataType::order));
}

template <common::MarketData MarketDataType>
bool MarketDataDB<MarketDataType>::save(const std::string& file_name) const
{
    std::ofstream out_stream(file_name, std::ofstream::binary);
    boost::archive::binary_oarchive out_arch(out_stream);
    out_arch&*(this);
    return true;
}
// PRIVATE MANIPULATORS
template <common::MarketData MarketDataType>
template <class Archive>
void MarketDataDB<MarketDataType>::serialize(Archive&           archive,
                                             const unsigned int version)
{
    archive & d_symbol_to_data;
}

} // namespace databases
} // namespace crypto_trader

#endif // INCLUDED_MARKET_DATA_DB
