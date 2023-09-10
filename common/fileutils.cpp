#include "fileutils.h"

#include <spdlog/spdlog.h>

#include <fstream>
#include <sstream>


namespace crypto_trader {
namespace common {

void readFile(std::string *fileContents, const std::string_view &filepath)
{
    std::ifstream file;
    file.open(filepath);

    if (file.is_open()) {
        while (file) {
            std::getline(file, *fileContents);
        }
    }
    else {
        spdlog::error("Couldn't open file {}", filepath);
    }
}

void readJsonFile(json *parsedJson, const std::string_view &filepath)
{
    std::string contents;
    readFile(&contents, filepath);
    try {
        *parsedJson = json::parse(contents);
    }
    catch (const json::parse_error& e) {
        std::stringstream ss;
        ss << e.what();
        spdlog::error("Unable to parse json {} with error: {}",
                      contents,
                      ss.str());
    }
}

} // closing namespace common
} // closing namespace crypto_trader
