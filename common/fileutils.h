#ifndef INCLUDED_FILE_UTLILS
#define INCLUDED_FILE_UTLILS

#include <nlohmann/json.hpp>

#include <atomic>
#include <memory>
#include <string>

namespace crypto_trader {
namespace common {

using json = nlohmann::json;

enum class ReadFileOptions {
    ALL_CONTENTS,
    LAST_LINE
}; // enum class ReadFileOptions

struct MonitorConfig {
    int                                d_inotify_fd;
    const char                        *d_trapFilePath;
    std::shared_ptr<std::atomic<bool>> d_isRunning;
}; // struct MonitorConfig

int readFile(std::string           *fileContents,
             const std::string    & filepath,
             const ReadFileOptions& readFileptions);
int readJsonFile(json *parsedJson, const std::string& filepath);

int createTrapFile(const char *trapFilePath);

int removeTrapFile(const MonitorConfig& config);

void monitorTrapFile(const MonitorConfig& config);

int handleInotifyEvents(const MonitorConfig& config);

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_FILE_UTLILS
