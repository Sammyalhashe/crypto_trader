#include "fileutils.h"

#include <spdlog/spdlog.h>

#include <fstream>
#include <sstream>


namespace crypto_trader {
namespace common {

int readFile(std::string *fileContents, const std::string &filepath)
{
    std::ifstream file;
    file.open(filepath);

    if (file.is_open()) {
        std::string line;
        while (file) {
            std::getline(file, line);
            *fileContents += line;
        }
    }
    else {
        spdlog::error("Couldn't open file {}", filepath);
        return -1;
    }
    return 0;
}

int readJsonFile(json *parsedJson, const std::string &filepath)
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
        return -1;
    }
    return 0;
}

} // closing namespace common
} // closing namespace crypto_trader
