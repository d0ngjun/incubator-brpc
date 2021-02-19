#ifndef COLOR_LOG_CLIENT_H
#define COLOR_LOG_CLIENT_H
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#include "butil/concurrentqueue.h"

#include "butil/iobuf.h"
#include "butil/logging.h"
#include "butil/strings/string_piece.h"
#include "butil/memory/singleton.h"
#include "brpc/channel.h"

DECLARE_bool(lockfree_queue_or_not);

namespace logging {
class ColorLogClient {
public:
    ~ColorLogClient();
    int Init(const std::string& module_name, const std::string& color_srv_name);
    int SendLog(
        int severity, 
        const char* file_name, 
        int line,
        const char* func,
        butil::IOBuf&& log_content);
    static ColorLogClient* GetInstance();
	int SendLogWithLock(
        int severity, 
        const char* file_name, 
        int line,
        const char* func,
        const butil::StringPiece& log_content);   
    int SendLogLockFree(
        int severity, 
        const char* file_name, 
        int line,
        const char* func,
        butil::IOBuf&& log_content);
private:
    ColorLogClient();
    ColorLogClient(const ColorLogClient&) = delete;
    ColorLogClient& operator=(const ColorLogClient&) = delete;

    void Run();
    void RunWithLock();
    void RunLockFree();
    void GetLogPrefix(int severity, std::shared_ptr<butil::IOBuf>& buf_t);

    bool ConnectLogStream();
    void SendLogToSvr(std::shared_ptr<butil::IOBuf>& log_buf);
    bool IsLogURLChange() const;
    std::string GetUri();
    friend struct DefaultSingletonTraits<ColorLogClient>;

private:
 struct  ColorLogWrapper {
    int level = 0;
    std::shared_ptr<butil::IOBuf> content;
 };

private:
    brpc::Channel channel_;
    std::string url_;
    bool running_;
    
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<std::shared_ptr<butil::IOBuf>> working_queue_;

    std::thread thread2_;
    moodycamel::ConcurrentQueue<std::shared_ptr<ColorLogWrapper>> pending_log_;

    time_t last_hour_;
    std::string local_ip_;
    uint16_t port_;
    std::string module_;
    uint32_t pid_;
    brpc::StreamId stream_id_;
    std::atomic<int> cached_log_size_;
    std::atomic<int> pending_log_size_;
};

}
#endif

