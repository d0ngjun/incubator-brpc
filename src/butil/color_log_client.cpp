#include <functional>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ostream>
#include <iomanip>
#include <chrono>

#include "gflags/gflags.h"
#include <bvar/bvar.h>
#include "butil/color_log_client.h"
#include "butil/location.h"
#include "butil/stream_log.pb.h"
#include "butil/color_log_header.h"
#include "brpc/policy/hasher.h"

DEFINE_string(logd_balance, "c_murmurhash", "log name load balance");
DEFINE_int32(stream_rpc_call_timeout, 200, "build stream rpc call timeout");
DEFINE_int32(max_color_log_buf_size, 32 * 1024 * 1024, 
    "maximum cached color log size during one second");
    
DEFINE_bool(lockfree_queue_or_not, true, "true: use loclfree_queue:moodycamel::ConcurrentQueue, false: use std::queue with std::mutex");
static bool validate_test_lockfree_queue_or_not(const char*, bool val) {
    return true;
}
const int ALLOW_UNUSED register_FLAGS_lockfree_queue_or_not = 
    ::GFLAGS_NS::RegisterFlagValidator(&FLAGS_lockfree_queue_or_not, validate_test_lockfree_queue_or_not);

namespace logging {
static std::string GetLocalIP(int family) {
    struct ifaddrs *addrs = nullptr;
    char buf[NI_MAXHOST] = { 0 };

    if (getifaddrs(&addrs) < 0) {
        PLOG(ERROR) << "call getifaddrs failed with error msg ";
        return "";
    }

    for (struct ifaddrs* ifa = addrs; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        if ((ifa->ifa_flags & IFF_UP) == 0) {
            continue;
        }

        if (family != ifa->ifa_addr->sa_family) {
            continue;
        }

        if (IFF_LOOPBACK & ifa->ifa_flags) {
            continue;
        }

        if (IFF_POINTOPOINT & ifa->ifa_flags) {
            continue;
        }

        if (AF_INET == ifa->ifa_addr->sa_family) {
            const struct sockaddr_in* addr4 = 
                reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr);
            if (inet_ntop(ifa->ifa_addr->sa_family,
                    &(addr4->sin_addr),
                    buf, NI_MAXHOST) != nullptr) {
                freeifaddrs(addrs);
                return buf;
            } else {
                PLOG(ERROR) << "call inet_ntop failed with error msg ";
            }
        } else if (AF_INET6 == ifa->ifa_addr->sa_family) {
            const struct sockaddr_in6* addr6 = 
                reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_addr);
            if (IN6_IS_ADDR_MULTICAST(&addr6->sin6_addr)) {
                continue;
            }

            if (IN6_IS_ADDR_LINKLOCAL(&addr6->sin6_addr)) {
                continue;
            }

            if (IN6_IS_ADDR_LOOPBACK(&addr6->sin6_addr)) {
                continue;
            }

            if (IN6_IS_ADDR_UNSPECIFIED(&addr6->sin6_addr)) {
                continue;
            }

            if (IN6_IS_ADDR_SITELOCAL(&addr6->sin6_addr)) {
                continue;
            }

            if (inet_ntop(ifa->ifa_addr->sa_family,
                    &(addr6->sin6_addr),
                    buf, NI_MAXHOST) != nullptr) {
                freeifaddrs(addrs);
                return buf;
            } else {
                PLOG(ERROR) << "call inet_ntop failed with error msg ";
            }
        }
    }

    freeifaddrs(addrs);
    return "";
}

ColorLogClient* ColorLogClient::GetInstance() {
    return Singleton<ColorLogClient>::get();
}

ColorLogClient::ColorLogClient() 
    : running_(false), 
      port_(0), 
      stream_id_(brpc::INVALID_STREAM_ID),
      cached_log_size_(0),
      pending_log_size_(0) {
    local_ip_ = GetLocalIP(AF_INET);
    pid_ = getpid();
}

ColorLogClient::~ColorLogClient() {
    if (running_) {
        running_ = false;
		//if (!FLAGS_lockfree_queue_or_not) 
        {            
            thread2_.join();
        } 
        //else 
        {
        	cond_.notify_one();
            thread_.join();
		}
    }

    if (stream_id_ != brpc::INVALID_STREAM_ID) {
        brpc::StreamClose(stream_id_);
        stream_id_ = brpc::INVALID_STREAM_ID;
    }
}

std::string LogLevelToName(int severity) {
    switch (severity) {
    case 0:
        return "kernel";
    case 1:
        return "debug";
    case 2:
        return "info";
    case 3:
        return "warn";
    case 4:
        return "error";
    case 5:
        return "fatal";
    default:
        break;
    }

    return "unknow";
}

int ColorLogClient::Init(
    const std::string& moudle_name, const std::string& color_srv_name) {
    brpc::ChannelOptions options;
    options.protocol = brpc::PROTOCOL_BAIDU_STD;
    options.timeout_ms = FLAGS_stream_rpc_call_timeout;
    options.connection_type = "single";

    module_ = moudle_name;
    if (channel_.Init(color_srv_name.c_str(), 
        FLAGS_logd_balance.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return -1;
    }

    thread_ = std::thread(std::bind(&ColorLogClient::RunWithLock, this));
    thread2_ = std::thread(std::bind(&ColorLogClient::RunLockFree, this));
    return 0;
}

int ColorLogClient::SendLog(
    int severity, 
    const char* file_name, 
    int line,
    const char* func,
    butil::IOBuf&& log_content) {
	
	if (FLAGS_lockfree_queue_or_not) {
	    return SendLogLockFree(severity, file_name, line, func, std::move(log_content));
	} else {
		//return SendLogWithLock(severity, file_name, line, func, std::move(log_content));
	}

    return -1;
}

int ColorLogClient::SendLogWithLock(
    int severity, 
    const char* file_name, 
    int line,
    const char* func,
    const butil::StringPiece& log_content) {
    
    ColorLogHeader* tls_header = ColorLogHeader::bls_color_log_header();
    if (tls_header == nullptr || !tls_header->log_coloring()) {
        return 0;
    }

    // discard log when cached log size reach 
    // FLAGS_max_color_log_buf_size bytes within one second to 
    // protect process from run out of memory
    if (cached_log_size_ >= FLAGS_max_color_log_buf_size) {
        return 0;
    }

    time_t t = time(nullptr);
    struct tm local_tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr };
    localtime_r(&t, &local_tm);
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    // log line format:
    // [info][2018-10-22 16:28:20.450][file:line][func][TRACE-19878][DEV-8766][UIN-98667][WX-8786656][BK-23466]log_content
    butil::IOBufBuilder buf;
    const char prev_fill = buf.fill('0');
    buf << '[';
    buf << LogLevelToName(severity) << ']';

    buf << '[';
    buf << std::setw(4) << local_tm.tm_year + 1900 << '-'
        << std::setw(2) << local_tm.tm_mon + 1 << '-'
        << std::setw(2) << local_tm.tm_mday << ' '
        << std::setw(2) << local_tm.tm_hour << ':'
        << std::setw(2) << local_tm.tm_min << ':'
        << std::setw(2) << local_tm.tm_sec << '.'
        << std::setw(4) << tv.tv_usec / 100 << ']';

    buf.fill(prev_fill);
    char* file = basename((char*)file_name);
    buf << '[' << file << ':' << line << ']';

    buf << '[' << func << ']';
    buf << "[TRACE-" << tls_header->trace_id() << ']';

    if (!tls_header->devid().empty()) {
        buf << "[DEV-" << tls_header->devid() << ']';
    }

    if (tls_header->uin() != 0) {
        buf << "[UIN-" << tls_header->uin() << ']';
    }

    if (!tls_header->wxopenid().empty()) {
        buf << "[WX-" << tls_header->wxopenid() << ']';
    }

    buf << "[BK-" << tls_header->bucketid() << ']';
    buf << log_content << '\n';
    
    std::shared_ptr<butil::IOBuf> log_buf = std::make_shared<butil::IOBuf>();
    buf.move_to(*log_buf);

    std::lock_guard<std::mutex> lock(mutex_);
    working_queue_.push(log_buf);
    cached_log_size_ += log_buf->size();
    if (working_queue_.size() > 1000) {
        cond_.notify_one();
    }

    return 0;
}

const int LOG_NUM_OF_SEVERITIES = 6;
const std::string log_severity_names[LOG_NUM_OF_SEVERITIES + 1] = {
    "kernel", "debug", "info", "warn", "error", "fatal", "unknow" };

const std::string& LogSeveritylToName(int severity) {
    if (severity >= 0 && severity < LOG_NUM_OF_SEVERITIES) {
        return log_severity_names[severity];
    } else {
        return log_severity_names[LOG_NUM_OF_SEVERITIES];
    }
}

int ColorLogClient::SendLogLockFree(
    int severity, 
    const char* file_name, 
    int line,
    const char* func,
    butil::IOBuf&& log_content) {
    
    ColorLogHeader* tls_header = ColorLogHeader::bls_color_log_header();
    if (tls_header == nullptr || !tls_header->log_coloring()) {
        return 0;
    }

    // discard log when cached log size reach 
    // FLAGS_max_color_log_buf_size bytes within one second to 
    // protect process from run out of memory
    if (pending_log_size_ >= FLAGS_max_color_log_buf_size) {
        return 0;
    }

    // log line format:
    // [2018-10-22 16:28:20.450][info][file:line][func][TRACE-19878][DEV-8766][UIN-98667][WX-8786656][BK-23466]log_content

    butil::IOBufBuilder buf;

    char* file = basename((char*)file_name);
    buf << '[' << file << ':' << line << ']';

    buf << '[' << func << ']';
    buf << "[TRACE-" << tls_header->trace_id() << ']';

    if (!tls_header->devid_iobuf().empty()) {
        buf << "[DEV-" << tls_header->devid_iobuf() << ']';
    }

    if (tls_header->uin() != 0) {
        buf << "[UIN-" << tls_header->uin() << ']';
    }

    if (!tls_header->wxopenid_iobuf().empty()) {
        buf << "[WX-" << tls_header->wxopenid_iobuf() << ']';
    }

    buf << "[BK-" << tls_header->bucketid() << ']';
    
    std::shared_ptr<butil::IOBuf> log_buf = std::make_shared<butil::IOBuf>(); //todo 复用之
    buf.move_to(*log_buf);
    log_buf->append(log_content);
    log_buf->push_back('\n');
          
    std::shared_ptr<ColorLogWrapper> log_wrapper = std::make_shared<ColorLogWrapper>();
    log_wrapper->level = severity;
    log_wrapper->content = log_buf;

    while (!pending_log_.enqueue(log_wrapper)) {
    }
    pending_log_size_ += log_wrapper->content->size();
	
    return 0;
}

void ColorLogClient::Run() {
	if (FLAGS_lockfree_queue_or_not) {
		RunLockFree();
	} else {
		RunWithLock();
	}
}

void ColorLogClient::RunWithLock() {
    running_ = true;
    while (running_) {
        std::queue<std::shared_ptr<butil::IOBuf>> q;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (working_queue_.empty() && running_) {
                cond_.wait_for(lock, std::chrono::microseconds(500));
            }

            q.swap(working_queue_);
            cached_log_size_ = 0;
        }
        
        while (!q.empty()) {
            SendLogToSvr(q.front());
            q.pop();
        }
        
    }

    // send the rest of data in queue to server
    std::unique_lock<std::mutex> lock(mutex_);
    while (!working_queue_.empty()) {
        SendLogToSvr(working_queue_.front());
        working_queue_.pop();
    }
}

void ColorLogClient::RunLockFree() {
    running_ = true;
    std::shared_ptr<ColorLogWrapper> log_wrapper;
    std::shared_ptr<butil::IOBuf> log_buf = std::make_shared<butil::IOBuf>();
    while (running_) {
        while ((pending_log_.size_approx() == 0) && running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        while (pending_log_.try_dequeue(log_wrapper)) {
            GetLogPrefix(log_wrapper->level, log_buf);
            log_buf->append(*(log_wrapper->content));
            SendLogToSvr(log_buf);
            pending_log_size_ -= log_wrapper->content->size();
            log_buf->clear();
            log_wrapper->content->clear();
        }    
    }

    // send the rest of data in queue to server
    while (pending_log_.try_dequeue(log_wrapper)) { 
        GetLogPrefix(log_wrapper->level, log_buf);   
        log_buf->append(*(log_wrapper->content));
        SendLogToSvr(log_buf);
        log_buf->clear();
        log_wrapper->content->clear();
    }
}

void ColorLogClient::GetLogPrefix(int severity, std::shared_ptr<butil::IOBuf>& log_buf) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm local_tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr };
    localtime_r(&tv.tv_sec, &local_tm);

    // log line format:
    // [info][2018-10-22 16:28:20.450][file:line][func][TRACE-19878][DEV-8766][UIN-98667][WX-8786656][BK-23466]log_content
    butil::IOBufBuilder buf;
    const char prev_fill = buf.fill('0');
            
    buf << '[';
    buf << LogSeveritylToName(severity) << ']';    

    buf << '[';
    buf << std::setw(4) << local_tm.tm_year + 1900 << '-'
        << std::setw(2) << local_tm.tm_mon + 1 << '-'
        << std::setw(2) << local_tm.tm_mday << ' '
        << std::setw(2) << local_tm.tm_hour << ':'
        << std::setw(2) << local_tm.tm_min << ':'
        << std::setw(2) << local_tm.tm_sec << '.'
        << std::setw(4) << tv.tv_usec / 100 << ']';

    buf.fill(prev_fill);
    buf.move_to(*log_buf);
}

void ColorLogClient::SendLogToSvr(std::shared_ptr<butil::IOBuf>& log_buf) {
    if (IsLogURLChange()) {
        url_ = GetUri();
        brpc::StreamClose(stream_id_);
        stream_id_ = brpc::INVALID_STREAM_ID;
    }

    for (int retry = 0; retry < 4; ++retry) {
        if (stream_id_ == brpc::INVALID_STREAM_ID) {
            if (!ConnectLogStream()) {
                std::this_thread::sleep_for(std::chrono::microseconds(retry * 10));
                continue;
            }
        }

        int ret = brpc::StreamWrite(stream_id_, *log_buf);
        if (ret == 0) { 
            return;
        } else if (ret == EINVAL) {
             stream_id_ = brpc::INVALID_STREAM_ID;
             LOG(ERROR) << "stream probably closed, id = " << stream_id_;
             continue;
        } else {
            LOG(ERROR) << "write to stream failed, id = " 
                       << stream_id_ 
                       << ", errno = " 
                       << ret;
            std::this_thread::sleep_for(std::chrono::microseconds(retry * 10));
        }
    }
}

bool ColorLogClient::IsLogURLChange() const {
    time_t cur_time = time(nullptr);
    if ((cur_time / 3600 * 3600) == last_hour_) {
        return false;
    }
    return true;
}

std::string ColorLogClient::GetUri() {
    time_t cur_time = time(nullptr);
    last_hour_ = cur_time / 3600 * 3600;
    struct tm local_tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr };
    localtime_r(&cur_time, &local_tm);
    
    //log uri format:/module_name/log2018102216.log.[pid][ip:port]
    std::ostringstream os;
    const char prev_fill = os.fill('0');
    os << "/" << module_ << "/log"
       << std::setw(4) << local_tm.tm_year + 1900
       << std::setw(2) << local_tm.tm_mon + 1
       << std::setw(2) << local_tm.tm_mday
       << std::setw(2) << local_tm.tm_hour
       << ".log.[" << pid_ << "][" << local_ip_ << ":" << port_ << "]";
    os.fill(prev_fill);
    return os.str();
}

bool ColorLogClient::ConnectLogStream() {
    if (stream_id_ == brpc::INVALID_STREAM_ID) {
        brpc::Controller cntl;
        cntl.set_request_code(brpc::policy::MurmurHash32(local_ip_.data(), local_ip_.size()));
        if (brpc::StreamCreate(&stream_id_, cntl, nullptr) != 0) {
            LOG(ERROR) << "Fail to create stream";
            return false;
        }
        LOG(INFO) << "create stream success, id = " << stream_id_;
        qqnews::ConnectStreamRequest req;
        qqnews::ConnectStreamResponse rsp;
        qqnews::LogService_Stub service_stub(&channel_);
        req.set_url(url_);
        service_stub.ConnectStream(&cntl, &req, &rsp, nullptr);
        if (cntl.Failed()) {
            brpc::StreamClose(stream_id_);
            stream_id_ = brpc::INVALID_STREAM_ID;
            LOG(ERROR) << "Fail to connect stream, " << cntl.ErrorText();
            return false;
        }
    }

    return true;
}

}

