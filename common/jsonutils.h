#ifndef INCLUDED_JSON_UTILS
#define INCLUDED_JSON_UTILS

#include <nlohmann/json.hpp>

namespace crypto_trader {
namespace common {

inline nlohmann::json value_or(const nlohmann::json& json,
                               const std::string&    key,
                               const nlohmann::json& def)
{
    return json.contains(key) ? json[key] : def;
}

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_JSON_UTILS
