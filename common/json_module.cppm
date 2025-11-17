module;

#include <nlohmann/json.hpp>

export module json_module;

export namespace json_module {

inline nlohmann::json value_or(const nlohmann::json& json,
                               const std::string&    key,
                               const nlohmann::json& def)
{
    return json.contains(key) ? json[key] : def;
}

}; // namespace json_module
