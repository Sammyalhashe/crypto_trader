// market_data_db.h

#ifndef INCLUDED_MARKET_DATA_DB
#define INCLUDED_MARKET_DATA_DB

#include "../common/serialization.h"
#include "../common/types.h"

#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace crypto_trader {
namespace databases {

template <common::MarketData MarketDataType>
class MarketDataDB {
  private:
    // PRIVATE DATA
    std::unordered_map<std::string, std::vector<MarketDataType>>
        d_symbol_to_data;

    mutable std::shared_mutex d_mutex;       // Thread-safe read/write access
    size_t d_max_entries_per_symbol = 10000; // Default: keep last 10k entries

  public:
    // CREATORS
    MarketDataDB() = default;
    explicit MarketDataDB(size_t max_entries)
    : d_max_entries_per_symbol(max_entries)
    {
    }
    ~MarketDataDB() = default;

    // Delete copy operations (mutex is non-copyable)
    MarketDataDB(const MarketDataDB&)            = delete;
    MarketDataDB& operator=(const MarketDataDB&) = delete;

    // MANIPULATORS
    void add_data(const std::string&    symbol,
                  const MarketDataType& market_data);

    bool load(const std::string& file_name);

    void set_max_entries(size_t max_entries)
    {
        std::unique_lock lock(d_mutex);
        d_max_entries_per_symbol = max_entries;
    }

    void prune_old_data(const std::string&                        symbol,
                        const typename MarketDataType::Timestamp& cutoff_ts);

    // ACCESSORS
    std::vector<MarketDataType>
    get_data(const std::string&                        symbol,
             const typename MarketDataType::Timestamp& min_ts,
             const typename MarketDataType::Timestamp& max_ts) const;

    size_t size(const std::string& symbol) const;

    bool save(const std::string& file_name) const;

  private:
    // PRIVATE MANIPULATORS
    template <class Archive>
    void serialize(Archive& archive, const unsigned int version);

    void prune_if_needed(std::vector<MarketDataType>& data_vec);

    // FRIENDS
    friend class boost::serialization::access;
}; // MarketDataDB

// ============================================================================
//                          INLINE DEFINITIONS
// ============================================================================

// PUBLIC MANIPULATORS
template <common::MarketData MarketDataType>
void MarketDataDB<MarketDataType>::add_data(const std::string&    symbol,
                                            const MarketDataType& market_data)
{
    std::unique_lock lock(d_mutex); // Exclusive write lock

    if (!d_symbol_to_data.contains(symbol)) {
        d_symbol_to_data[symbol] = std::vector<MarketDataType>();
        d_symbol_to_data[symbol].reserve(d_max_entries_per_symbol);
    }

    auto& data_vec = d_symbol_to_data[symbol];

    // OPTIMIZATION: Happy path for chronological data (O(1) instead of O(n))
    if (data_vec.empty() ||
        MarketDataType::order(data_vec.back(), market_data))
    {
        data_vec.push_back(market_data); // Fast path
    }
    else {
        // Slow path: out-of-order data (rare in live feeds)
        auto insert_pos = std::upper_bound(data_vec.begin(),
                                           data_vec.end(),
                                           market_data,
                                           MarketDataType::order);
        data_vec.insert(insert_pos, market_data);
    }

    // Auto-prune if exceeding max size
    prune_if_needed(data_vec);
}

template <common::MarketData MarketDataType>
bool MarketDataDB<MarketDataType>::load(const std::string& file_name)
{
    std::unique_lock lock(d_mutex); // Exclusive write lock

    try {
        std::ifstream in_stream(file_name, std::ios::binary);
        if (!in_stream.is_open()) {
            return false;
        }

        boost::archive::binary_iarchive in_arch(in_stream);
        in_arch & d_symbol_to_data; // Fixed: removed dereference operator

        return in_stream.good();
    }
    catch (const boost::archive::archive_exception& e) {
        // Log error in production:
        spdlog::error("Load failed: {}", e.what());
        d_symbol_to_data.clear(); // Reset to clean state
        return false;
    }
    catch (const std::exception& e) {
        d_symbol_to_data.clear();
        return false;
    }
}

template <common::MarketData MarketDataType>
void MarketDataDB<MarketDataType>::prune_old_data(
    const std::string&                        symbol,
    const typename MarketDataType::Timestamp& cutoff_ts)
{
    std::unique_lock lock(d_mutex);

    if (!d_symbol_to_data.contains(symbol)) {
        return;
    }

    auto& data_vec = d_symbol_to_data[symbol];

    // Create temporary object for comparison
    MarketDataType cutoff_data;
    if constexpr (requires { cutoff_data.d_sequence; }) {
        cutoff_data.d_sequence = cutoff_ts;
    }
    else if constexpr (requires { cutoff_data.timestamp; }) {
        cutoff_data.timestamp = cutoff_ts;
    }

    auto cutoff_it = std::lower_bound(
        data_vec.begin(), data_vec.end(), cutoff_data, MarketDataType::order);

    data_vec.erase(data_vec.begin(), cutoff_it);
}

// PUBLIC ACCESSORS
template <common::MarketData MarketDataType>
std::vector<MarketDataType> MarketDataDB<MarketDataType>::get_data(
    const std::string&                        symbol,
    const typename MarketDataType::Timestamp& min_ts,
    const typename MarketDataType::Timestamp& max_ts) const
{
    std::shared_lock lock(d_mutex); // Shared read lock

    if (!d_symbol_to_data.contains(symbol)) {
        return {};
    }

    const auto& data_vec = d_symbol_to_data.at(symbol);

    // FIX: Create temporary objects for comparison
    MarketDataType min_data, max_data;

    // Handle both d_sequence and timestamp field names
    if constexpr (requires { min_data.d_sequence; }) {
        min_data.d_sequence = min_ts;
        max_data.d_sequence = max_ts;
    }
    else if constexpr (requires { min_data.timestamp; }) {
        min_data.timestamp = min_ts;
        max_data.timestamp = max_ts;
    }

    auto start_it = std::lower_bound(
        data_vec.begin(), data_vec.end(), min_data, MarketDataType::order);

    auto end_it = std::upper_bound(
        data_vec.begin(), data_vec.end(), max_data, MarketDataType::order);

    return std::vector<MarketDataType>(start_it, end_it);
}

template <common::MarketData MarketDataType>
size_t MarketDataDB<MarketDataType>::size(const std::string& symbol) const
{
    std::shared_lock lock(d_mutex);

    if (!d_symbol_to_data.contains(symbol)) {
        return 0;
    }
    return d_symbol_to_data.at(symbol).size();
}

template <common::MarketData MarketDataType>
bool MarketDataDB<MarketDataType>::save(const std::string& file_name) const
{
    std::shared_lock lock(d_mutex); // Shared read lock

    try {
        std::ofstream out_stream(file_name, std::ios::binary);
        if (!out_stream.is_open()) {
            return false;
        }

        boost::archive::binary_oarchive out_arch(out_stream);
        out_arch & d_symbol_to_data; // Fixed: removed dereference operator

        return out_stream.good();
    }
    catch (const boost::archive::archive_exception& e) {
        // Log error in production:
        spdlog::error("Save failed: {}", e.what());
        return false;
    }
    catch (const std::exception& e) {
        return false;
    }
}

// PRIVATE MANIPULATORS
template <common::MarketData MarketDataType>
template <class Archive>
void MarketDataDB<MarketDataType>::serialize(Archive&           archive,
                                             const unsigned int version)
{
    archive & d_symbol_to_data;
    // Note: d_mutex is not serialized (reconstructed on load)
}

template <common::MarketData MarketDataType>
void MarketDataDB<MarketDataType>::prune_if_needed(
    std::vector<MarketDataType>& data_vec)
{
    if (data_vec.size() > d_max_entries_per_symbol) {
        size_t excess = data_vec.size() - d_max_entries_per_symbol;
        data_vec.erase(data_vec.begin(), data_vec.begin() + excess);
    }
}

} // namespace databases
} // namespace crypto_trader

#endif // INCLUDED_MARKET_DATA_DB
