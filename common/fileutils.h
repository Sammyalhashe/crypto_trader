#ifndef INCLUDED_FILE_UTLILS
#define INCLUDED_FILE_UTLILS

#include <nlohmann/json.hpp>

#include <string>

namespace crypto_trader {
namespace common {

using json = nlohmann::json;

void readFile(std::string             *fileContents,
                     const std::string&  filepath);
void readJsonFile(json *parsedJson, const std::string& filepath);

} // closing namespace common
} // closing namespace crypto_trader

#endif // INCLUDED_FILE_UTLILS
