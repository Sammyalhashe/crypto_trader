#ifndef INCLUDED_FILE_UTLILS
#define INCLUDED_FILE_UTLILS

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

namespace crypto_trader {
namespace common {

using json = nlohmann::json;

static void readFile(std::string             *fileContents,
                     const std::string_view&  filepath);
static void readJsonFile(json *parsedJson, const std::string_view& filepath);

} // closing namespace common
} // closing namespace crypto_trader

#endif // INCLUDED_FILE_UTLILS
