#include <sstream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "butil/log_file.h"
#include "butil/fd_guard.h"
#include "butil/logging.h"

namespace logging {

LogFile::LogFile() : log_file_(nullptr), last_roll_time_(0) {
    RollFile();
}

LogFile::~LogFile() {
    if (log_file_) {
        // fclose will flush the buffered data to disk
        fclose(log_file_);
        log_file_ = nullptr;
    }
}

void LogFile::Write(const char* logline, int len, bool flush) {
    if (log_file_ && logline) {
        // it looks like glibc's fwrite have an internal lock,
        // we don't need it here
        // http://www.gnu.org/software/libc/manual/html_node/Block-Input_002fOutput.html
        fwrite_unlocked(logline, len, 1, log_file_);
        if (flush) {
            fflush(log_file_);
        }
        RollFile();
    }
}

void LogFile::Flush() {
    if (!log_file_) {
        return;
    }

    fflush(log_file_);
}

void LogFile::RollFile()
{
    time_t start = time(nullptr) / 3600 * 3600;
    if (start != last_roll_time_) {
        if (log_file_) {
            fclose(log_file_);
            log_file_ = nullptr;
        }
        
        std::string path = GetFullLogPath().value();
        log_file_ = fopen(path.c_str(), "ae");
        if (!log_file_) {
            std::cerr << "Fail to open " << path << ",error msg : " << strerror(errno);
            return;
        }
        last_roll_time_ = start;
    }
}

std::string LogFile::GetLogFileName() {
    std::string filename = GetProcessName();
    char timebuf[32];
    struct tm tm;
    time_t now = time(nullptr);
    localtime_r(&now, &tm);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;

    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, ".%d", getpid());
    filename += pidbuf;

    filename += ".log";
    return filename;
}

butil::FilePath LogFile::GetFullLogPath() {
    return butil::FilePath(FLAGS_log_directory).Append(GetLogFileName());
}

}
