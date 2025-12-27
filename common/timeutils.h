
#ifndef INCLUDED_TIME_UTILS
#define INCLUDED_TIME_UTILS

#include "types.h"

#include <chrono>

namespace crypto_trader {
namespace common {

// Get current time as milliseconds since Unix epoch
inline int64_t getCurrentTimestampMs()
{

    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

template <common::MarketData T>
T::Timestamp getCurrentTimestampMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

inline int64_t parseISO8601ToMillis(const std::string& iso8601)
{
    int year, month, day, hour, minute, second, micros = 0;

    // Parse: "2025-11-30T20:27:00.123456Z"
    int parsed = std::sscanf(iso8601.c_str(),
                             "%d-%d-%dT%d:%d:%d.%d",
                             &year,
                             &month,
                             &day,
                             &hour,
                             &minute,
                             &second,
                             &micros);

    if (parsed < 6) { // At least date/time required
        throw std::runtime_error("Invalid ISO 8601 format: " + iso8601);
    }

    // Build tm structure
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon  = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min  = minute;
    tm.tm_sec  = second;

// Convert to Unix timestamp (UTC)
#ifdef _WIN32
    std::time_t time = _mkgmtime(&tm);
#else
    std::time_t time = timegm(&tm);
#endif

    // Convert to milliseconds
    int64_t milliseconds = static_cast<int64_t>(time) * 1000;

    // Add fractional seconds (microseconds -> milliseconds)
    if (parsed == 7) {
        // Adjust based on how many digits were in the fractional part
        // Coinbase sends 6 digits (microseconds)
        milliseconds += micros / 1000;
    }

    return milliseconds;
}

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_TIME_UTILS
