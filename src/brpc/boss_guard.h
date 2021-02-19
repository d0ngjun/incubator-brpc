#ifndef BOSS_GUARD_HH
#define BOSS_GUARD_HH

#include <string>

namespace google {
namespace protobuf {
class Message;
}
}

namespace brpc {
    
const std::string& GetUnderscoredModuleName();

class Controller;
class Channel;
class RedisRequest;
class WupRequest;

class BossGuard {
public:
    BossGuard();
    BossGuard(Controller* cntl, 
              Channel* channel,
              const google::protobuf::Message* request,
              const int64_t begin_time_us);
    void Mcall(Controller* cntl);
    ~BossGuard();

private:
    void ParseRedisCommand(const RedisRequest& req);
    void ParseHttpUri(Controller* cntl, std::string* host_uri, std::string* remote_uri);
    void ParseWupCommand(const WupRequest& req);

private:
    std::string remote_name_;
    std::string remote_uri_;
    std::string host_uri_;
    int64_t begin_time_us_;
    bool enable_;
};

class BossHostGuard {
public:
    explicit BossHostGuard(Controller* in);
    void Mcall();
    ~BossHostGuard();
private:
    void ParseHTTPUri();

private:
    Controller* cntl_;
    std::string host_uri_;
    std::string remote_ip_;
    int code_;
};

}
#endif
