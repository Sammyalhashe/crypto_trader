#include "fileutils.h"

#include <spdlog/spdlog.h>

#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <sstream>

#include <unistd.h>

namespace crypto_trader {
namespace common {

namespace {

bool isEmptyLine(const std::string& line)
{
    return line == "" || line == "\n" || line == "\r" || line == "\n\r";
}

} // namespace

int readFile(std::string           *fileContents,
             const std::string&     filepath,
             const ReadFileOptions& readFileOptions)
{
    std::ifstream file;
    file.open(filepath);

    if (file.is_open()) {
        std::string line;
        while (file) {
            std::getline(file, line);
            if (readFileOptions == ReadFileOptions::ALL_CONTENTS) {
                *fileContents += line;
            }
            else if (!isEmptyLine(line) &&
                     readFileOptions == ReadFileOptions::LAST_LINE)
            {
                *fileContents = line;
            }
        }
    }
    else {
        spdlog::error("Couldn't open file {}", filepath);
        return -1;
    }
    return 0;
}

int readJsonFile(json *parsedJson, const std::string& filepath)
{
    std::string contents;
    readFile(&contents, filepath, ReadFileOptions::ALL_CONTENTS);
    try {
        *parsedJson = json::parse(contents);
    }
    catch (const json::parse_error& e) {
        std::stringstream ss;
        ss << e.what();
        spdlog::error(
            "Unable to parse json {} with error: {}", contents, ss.str());
        return -1;
    }
    return 0;
}

#ifdef __linux__
int createTrapFile(const char *trapFilePath)
{
    int fd = open(trapFilePath,
                  O_CREAT,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0) {
        spdlog::error("unable to create trap file: {}", trapFilePath);
        return -1;
    }

    fd = inotify_init1(IN_NONBLOCK);

    if (fd < 0) {
        spdlog::error("error creating inotify instance");
        return -1;
    }

    int watch_fd = inotify_add_watch(fd, trapFilePath, IN_MODIFY);

    if (watch_fd < 0) {
        spdlog::error("Unable to create a watch descriptor for filepath: {}",
                      trapFilePath);

        if (errno == EACCES) {
            spdlog::error("EACCES");
        }
        else if (errno == EBADF) {
            spdlog::error("EDADF");
        }
        else if (errno == EEXIST) {
            spdlog::error("EEXIST");
        }
        else if (errno == EFAULT) {
            spdlog::error("EFAULT");
        }
        else if (errno == EINVAL) {
            spdlog::error("EINVAL");
        }
        else if (errno == ENOENT) {
            spdlog::error("ENOENT");
        }
        return -1;
    }

    return fd;
}

int removeTrapFile(const MonitorConfig& config)
{
    int rc = close(config.d_inotifyFd);
    if (rc < 0) {
        spdlog::error("unable to close file descriptor: {}",
                      config.d_inotifyFd);
        return -1;
    }
    rc = std::remove(config.d_trapFilePath);

    if (rc != 0) {
        spdlog::error("unable to remove the file: {}", config.d_trapFilePath);
        return -2;
    }
    return 0;
}

int handleInotifyEvents(const MonitorConfig& config)
{
    /* Some systems cannot read integer variables if they are not
       properly aligned. On other systems, incorrect alignment may
       decrease performance. Hence, the buffer used for reading from
       the inotify file descriptor should have the same alignment as
       struct inotify_event. */

    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    int                         i;
    ssize_t                     len;
    char                       *ptr;

    /* Loop while events can be read from inotify file descriptor. */
    for (;;) {
        len = read(config.d_inotifyFd, buf, sizeof buf);

        if (-1 == len && errno != EAGAIN) {
            spdlog::error("fatal: error occurred reading the trap file: {}",
                          config.d_inotifyFd);
            return -1;
        }

        if (-1 >= len) {
            break;
        }

        for (ptr = buf; ptr < buf + len;
             ptr += sizeof(inotify_event) + event->len)
        {
            event = (const struct inotify_event *)ptr;

            if (event->mask & IN_MODIFY) {
                SPDLOG_INFO("inotify_fd detected change in file: {}",
                            config.d_trapFilePath);
                std::string fileContents;
                readFile(&fileContents,
                         config.d_trapFilePath,
                         ReadFileOptions::LAST_LINE);
                std::stringstream ss;
                ss << fileContents;

                if (fileContents == "exit") {
                    *config.d_isRunning = false;
                }
            }
        }
    }

    return 0;
}

void monitorTrapFile(const MonitorConfig& config)
{
    SPDLOG_INFO("watching inotify_fd: {}", config.d_inotifyFd);

    if (config.d_inotifyFd < 0) {
        return;
    }

    constexpr int nfds = 1;
    pollfd        pfds[nfds];
    pfds[0].fd     = config.d_inotifyFd;
    pfds[0].events = POLLIN;

    int pollNum;

    while (*config.d_isRunning) {
        // last value is timeout
        pollNum = poll(pfds, nfds, -1);

        SPDLOG_INFO("pollNum: {}", pollNum);

        if (-1 == pollNum) {
            // interrupted system call just keep going
            if (errno == EINTR) {
                continue;
            }
            spdlog::error("error while polling inotify_fd: {}: {}",
                          config.d_inotifyFd,
                          errno);
            return;
        }

        if (pollNum > 0) {
            if (pfds[0].revents & POLLIN) {
                /* Inotify events are available */
                handleInotifyEvents(config);
            }
        }
    }
}

#endif // __linux__

#if TARGET_OS_MAC
void fsEventStreamCallback(ConstFSEventStreamRef         streamRef,
                           void                         *clientCallBackInfo,
                           size_t                        numEvents,
                           void                         *eventPaths,
                           const FSEventStreamEventFlags eventFlags[],
                           const FSEventStreamEventId    eventIds[])
{
    assert(clientCallBackInfo);
    const MonitorConfig& config = *(MonitorConfig *)clientCallBackInfo;
    int                  i;
    char               **paths = (char **)eventPaths;

    for (i = 0; i < numEvents; i++) {
        if (eventFlags[i] & kFSEventStreamEventFlagItemModified) {
            std::string fileContents;
            readFile(&fileContents,
                     config.d_trapFilePath,
                     ReadFileOptions::LAST_LINE);
            std::stringstream ss;
            ss << fileContents;

            SPDLOG_INFO("file contents: {}", ss.str());

            if (fileContents == "exit") {
                SPDLOG_INFO("stopping!");
                *config.d_isRunning = false;

                // stops the stream.
                // ensures the client callback will not be called again for the
                // stream.
                FSEventStreamRef ref = const_cast<FSEventStreamRef>(streamRef);
                FSEventStreamStop(ref);

                // Invalidates the stream.
                FSEventStreamInvalidate(ref);

                // Decrements the refcount on the stream.
                FSEventStreamRelease(ref);
            }
        }
    }
}

bool createEventStream(const MonitorConfig& config)
{
    CFStringRef mypath = __CFStringMakeConstantString(config.d_trapFilePath);
    CFArrayRef  pathsToWatch =
        CFArrayCreate(nullptr, (const void **)&mypath, 1, nullptr);
    FSEventStreamContext callbackInfo = {
        0, (void *)&config, nullptr, nullptr, nullptr};
    FSEventStreamRef stream;
    CFTimeInterval   latency = 0.5;

    SPDLOG_INFO("creating fseventstream...");
    stream = FSEventStreamCreate(nullptr,
                                 &fsEventStreamCallback,
                                 &callbackInfo,
                                 (CFArrayRef)pathsToWatch,
                                 kFSEventStreamEventIdSinceNow,
                                 (CFAbsoluteTime)latency,
                                 kFSEventStreamCreateFlagFileEvents);

    FSEventStreamScheduleWithRunLoop(
        stream,
        /* see here
         * https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Multithreading/RunLoopManagement/RunLoopManagement.html
         * for more details on the below arguments.
         */
        CFRunLoopGetCurrent(),
        kCFRunLoopDefaultMode);

    FSEventStreamStart(stream);

    CFRunLoopRun();
    return true;
}

#endif // TARGET_OS_MAC

} // namespace common
} // namespace crypto_trader
