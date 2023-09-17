#ifndef INCLUDED_MARKET_DATA_DB
#define INCLUDED_MARKET_DATA_DB

#include <unordered_map>
#include <vector>
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace crypto_trader {
namespace databases {

// TODO: add multithread support (mutexes)
// Read / Write lock per symbol? 
// Read / Write lock for the entire map
// Potentially set up each symbol beforehand to not lock the map.
// If locking map may as well have 1 lock
template<class MarketDataType>
class MarketDataDB {
public:
    MarketDataDB() { }

    ~MarketDataDB() { }

    void add_data(const std::string& symbol, const MarketDataType& market_data) noexcept {
        // TODO: ADD a happy path:
        // check if the last element is at a earlier time than the new dat with the ordering predicate.
        if (!d_symbol_to_data.contains(symbol)) {
            d_symbol_to_data[symbol] = std::vector<MarketDataType>();
        }

        auto& data_vec = d_symbol_to_data[symbol];
        data_vec.insert(
            std::upper_bound(
                data_vec.begin(),
                data_vec.end(),
                market_data,
                MarketDataType::order
            ),
            market_data
        );
    }


    std::vector<MarketDataType> get_data(const std::string& symbol,
            const MarketDataType::Timestamp& min_ts,
            const MarketDataType::Timestamp& max_ts) {
        auto& data_vec = d_symbol_to_data[symbol];
        return std::vector<MarketDataType>(
            std::lower_bound(
                data_vec.begin(),
                data_vec.end(),
                min_ts,
                MarketDataType::order
            ),
            std::upper_bound(
                data_vec.begin(),
                data_vec.end(),
                max_ts,
                MarketDataType::order
            )
        );
    }

    // TODO: make an error status type for saving and loading.
    bool save(const std::string& file_name) const {
        std::ofstream out_stream(file_name, std::ofstream::binary);
        boost::archive::binary_oarchive out_arch(out_stream);
        out_arch & *(this);
        return true;
    }

    bool load(std::string& file_name) {
        std::ifstream in_stream(file_name, std::ofstream::binary);
        boost::archive::binary_iarchive in_arch(in_stream);
        in_arch & *(this);
        return true;
    }
private:
    friend class boost::serialization::access;

    std::unordered_map<std::string, std::vector<MarketDataType>> d_symbol_to_data;
};

} // closing namespace databases
} // closing namespace crypto_trader


#endif
