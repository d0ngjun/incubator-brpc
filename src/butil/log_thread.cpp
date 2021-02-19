#include <chrono>
#include <functional>
#include "gflags/gflags.h"
#include "butil/log_thread.h"
#include "butil/log_file.h"

DEFINE_int32(max_log_buf_size, 32 * 1024 * 1024, 
    "maximum cached log size during one second");

namespace logging {

LogThread::LogThread() : running_(false), cached_log_size_(0) {
    thread_ = std::thread(std::bind(&LogThread::Run, this));
}

void LogThread::Append(std::string&& log) {
    // discard log when cached log size reach FLAGS_max_log_buf_size bytes 
    // within one second to protect process from run out of memory
    std::lock_guard<std::mutex> lock(mutex_);
    if (cached_log_size_ + int(log.size()) >= FLAGS_max_log_buf_size) {
        return;
    }
    cached_log_size_ += log.size();
    working_queue_.push_back(std::move(log));
    if (working_queue_.size() > 1000) {
        cond_.notify_one();
    }
}

void LogThread::Run() {
    running_ = true;
    LogFile file;
    while (running_) {
        std::vector<std::string> log_queue;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (working_queue_.empty() && running_) {
                cond_.wait_for(lock, std::chrono::seconds(1));
            }
            
            log_queue.swap(working_queue_);
            cached_log_size_ = 0;
        }

        for (auto it = log_queue.begin(); it != log_queue.end(); ++it) {
            file.Write(it->data(), it->size());
        }
        file.Flush();
    }

    // we might have data in working queue when log thread quit
    // flush working queue data to disk synchronously
    std::lock_guard<std::mutex> lock(mutex_);
    if (!working_queue_.empty()) {
        for (auto it = working_queue_.begin(); it != working_queue_.end(); ++it) {
            file.Write(it->data(), it->size());
        }
    }

    file.Flush();
}

}

