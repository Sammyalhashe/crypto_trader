#ifndef INCLUDED_FILE_UTLILS
#define INCLUDED_FILE_UTLILS

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
#ifdef __linux__
    int d_inotify_fd;
#endif // __linux__
    const char                        *d_trapFilePath;
    std::shared_ptr<std::atomic<bool>> d_isRunning;
}; // struct MonitorConfig

int readFile(std::string           *fileContents,
             const std::string    & filepath,
             const ReadFileOptions& readFileptions);
int readJsonFile(json *parsedJson, const std::string& filepath);

void handleCommand(const std::string  & command,
                   const MonitorConfig& config,
                   void                *context);

// COMMANDS
void handleExit(const std::vector<std::string>& args,
                const MonitorConfig           & config,
                void                           *context);
void handleRunAgainst(const std::vector<std::string>& args,
                      const MonitorConfig           & config,
                      void                           *context);

#ifdef __linux__
int createTrapFile(const char *trapFilePath);

int removeTrapFile(const MonitorConfig& config);

void monitorTrapFile(const MonitorConfig& config);

int handleInotifyEvents(const MonitorConfig& config);
#endif // __linux__

#ifdef TARGET_OS_MAC
void fsEventStreamCallback(ConstFSEventStreamRef         streamRef,
                           void                         *clientCallBackInfo,
                           size_t                        numEvents,
                           void                         *eventPaths,
                           const FSEventStreamEventFlags eventFlags[],
                           const FSEventStreamEventId    eventIds[]);

bool createEventStream(const MonitorConfig& config);
#endif // TARGET_OS_MAC

} // namespace common
} // namespace crypto_trader

#endif // INCLUDED_FILE_UTLILS
