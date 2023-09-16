#ifndef INCLUDED_MARKET_DATA_DB
#define INCLUDED_MARKET_DATA_DB

#include <unordered_map>
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace crypto_trader {
namespace databases {

template<MarketDataType>
class MarketDataDB {
public:
    MarketDataDB() { }

    ~MarketDataDB() { }

    void add_data(const std::string& symbol, const MarketDataType& market_data) noexcept {
        if (!data_vec.contains(symbol)) {
            data_vec[symbol] = std::vector<MarketDataType>();
        }
        // TODO: ADD a happy path:
        // check if the last element is at a earlier time than the new dat with the ordering predicate.
        auto& data_vec = d_symbol_to_data[symbol];
        data_vec[symbol].insert(
            std::upper_bound(
                data_vec.begin(),
                data_vec.end(),
                market_data,
                MarketDataType::order_predicate
            ),
            market_data
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

}
}

#endif
