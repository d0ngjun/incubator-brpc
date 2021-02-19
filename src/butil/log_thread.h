#ifndef LOG_THREAD_H
#define LOG_THREAD_H

#include <thread>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <memory>
#include "butil/macros.h"

namespace logging {

class LogThread
{
public:
    LogThread();
    ~LogThread() {
        Stop();
    }

    void Append(std::string&& log);
    void Stop() {
        if (running_) {
            running_ = false;
            cond_.notify_one();
            thread_.join();
        }
    }

    bool IsRunning() const {
        return running_;
    }
private:
    void Run();
    bool running_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::vector<std::string> working_queue_;
    int cached_log_size_;
    DISALLOW_COPY_AND_ASSIGN(LogThread);
};

}
#endif

