#ifndef INCLUDED_FILE_UTLILS
#define INCLUDED_FILE_UTLILS

#include <algorithm>
#include <nlohmann/json.hpp>

#include <atomic>
#include <memory>
#include <string>

#ifdef __linux__
#include <sys/inotify.h>
#include <sys/poll.h>
#elif __APPLE__
#include <TargetConditionals.h>
#ifdef TARGET_OS_MAC
#include <CoreServices/CoreServices.h>
#endif // TARGET_OS_MAC
#endif // __linux__

namespace crypto_trader {
namespace common {

using json = nlohmann::json;

enum class ReadFileOptions {
    ALL_CONTENTS,
    LAST_LINE
}; // enum class ReadFileOptions

struct MonitorConfig {
    using MtrapCb = std::function<void(std::istream& istream)>;
    using MtrapMap = std::unordered_map<std::string, MtrapCb>;
#ifdef __linux__
    // File descriptor for inotify events
    int                                d_inotify_fd;
#endif // __linux__
    // The path of the file watched for events
    const char                        *d_trapFilePath;
    MtrapMap                           d_mtrapMap;
    // The state of the app
    std::shared_ptr<std::atomic<bool>> d_isRunning;
}; // struct MonitorConfig


int createDirIfNotExists(const char* dirPath);

int createFile(const char* filepath);

int readFile(std::string           *fileContents,
             const std::string    & filepath,
             const ReadFileOptions& readFileptions);
int readJsonFile(json *parsedJson, const std::string& filepath);

#ifdef __linux__
int createTrapFile(const char *trapFilePath);

int removeTrapFile(const MonitorConfig& config);

void monitorTrapFile(MonitorConfig* config);

int handleInotifyEvents(const MonitorConfig& config);
#endif // __linux__

#ifdef TARGET_OS_MAC
void fsEventStreamCallback(
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[]);

bool createEventStream(const MonitorConfig& config);
#endif // TARGET_OS_MAC

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_FILE_UTLILS
