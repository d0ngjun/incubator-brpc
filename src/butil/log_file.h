#ifndef LOG_FILE_H
#define LOG_FILE_H

#include "butil/files/file.h"
#include "butil/macros.h"
#include "butil/files/file_path.h"

namespace logging {

class LogFile
{
public:
    LogFile();
    ~LogFile();

    void Write(const char* logline, int len, bool flush = false);
    void Flush();
private:
    static std::string GetLogFileName();
    static butil::FilePath GetFullLogPath();
    DISALLOW_COPY_AND_ASSIGN(LogFile);
    void RollFile();

    FILE* log_file_;
    time_t last_roll_time_;
};

}

#endif
