#include "brpc/boss_guard.h"

#include <map>
#include <string>
#include <utility>

#include "brpc/server.h"    
#include "brpc/controller.h"                // brpc::Controller
#include "brpc/channel.h"                   // brpc::Channel
#include "brpc/options.pb.h"                // brpc::PROTOCOL_HTTP
#include "brpc/redis.h"                     // brpc::RedisRequest
#include "brpc/wup.h"                       // brpc::WupRequest
#include "bvar/bvar.h"
#include "butil/strings/string_number_conversions.h"
#include "butil/strings/string_piece.h"     // butil::StringPiece
#include "butil/strings/string_split.h"
#include "prometheus/summary.h"
#include "prometheus/family_container.h"

#include "google/protobuf/descriptor.h"

#include <gflags/gflags.h>
#include <bossapi.h>

DEFINE_string(module_name, "odin", "the module name");
DEFINE_int32(mcall_project_id, 0, "mcall project id");
DEFINE_bool(mcall_enable, true, "use stop mcall report");
DEFINE_bool(mcall_with_bucket, false, "add bucket with mod");
DEFINE_bool(mcall_verbose, false, "verbose mcall return code");
DEFINE_bool(prometheus_enable, true, "prometheus monitor enable");
DEFINE_bool(split_expid, true, "prometheus use single expid");
DEFINE_bool(enable_collect_metric_expid, true, "enable_collect_metric_expid");
DEFINE_bool(enable_collect_remote_uri, false, "enable_collect_remote_uri");
DEFINE_bool(enable_collect_local_name, false, "enable_collect_local_name");
DEFINE_bool(enable_collect_local_uri, false, "enable_collect_local_uri");
DEFINE_bool(enable_collect_bucket, false, "enable_collect_bucket");
DEFINE_bool(enable_collect_channel, false, "enable_collect_channel");

DEFINE_double(hist_latency_p1, 1000.0, "First bucket for latency");
DEFINE_double(hist_latency_p2, 2000.0, "Second bucket for latency");
DEFINE_double(hist_latency_p3, 5000.0, "Third bucket for latency");
DEFINE_double(hist_latency_p4, 10000.0, "Forth bucket for latency");
DEFINE_double(hist_latency_p5, 20000.0, "Fifth bucket for latency");
DEFINE_double(hist_latency_p6, 50000.0, "sixth bucket for latency");

static bool validate_mcall_verbose(const char*, bool val) {
    return true;
}
const int ALLOW_UNUSED register_FLAGS_mcall_verbose = 
    ::GFLAGS_NS::RegisterFlagValidator(&FLAGS_mcall_verbose, validate_mcall_verbose);


#if BOSS_TEST
bvar::Adder<int> boss_call;
bvar::Adder<int> boss_host_call;
#endif

namespace brpc {

static pthread_once_t get_underscored_module_name = PTHREAD_ONCE_INIT;
static string underscored_module_name;

static void GetUnderscoredModuleNameOnce() {
    bvar::to_underscored_name(&underscored_module_name, FLAGS_module_name);
}

const std::string& GetUnderscoredModuleName() {
    if (underscored_module_name.empty()) {
        pthread_once(&get_underscored_module_name, GetUnderscoredModuleNameOnce);
    }
    
    return underscored_module_name;
}

const static std::string kDelimiter = "_";

static bool EnablePrometheusMetrics(const butil::StringPiece& uri) {
   if (uri.starts_with("/vars/") ||
       uri.starts_with("/js/") ||
       uri.starts_with("/metrics/") || 
       0 == uri.compare("/vars") ||
       0 == uri.compare("/js") ||
       0 == uri.compare("/metrics")) {
       return false;
   }
   return true;
}

static void GetHeader(brpc::Controller* cntl, const std::string& header_k, std::string& header_v) {
    const std::string* pheader = cntl->http_request().GetHeader(header_k);
    if (pheader) {
        header_v = *pheader;
    }
}

static void GetExpId(brpc::Controller* cntl, std::vector<std::string>* expids) {
    const std::string* pexpid = cntl->http_request().GetHeader("Exp-Id");
    string expid("000000");
    if (pexpid) {
        expid = *pexpid;
    }

    if (FLAGS_split_expid) {
        butil::SplitString(expid, '|', expids);
    } else {
        expids->push_back(expid);
    }
}

static void ReportPromethusHelper(const std::string& count_name, 
                                  const std::string& histogram_name,
                                  const std::map<std::string, std::string>& sub_labels,
                                  int64_t latency_us) {
    auto* counter_family = prometheus::FamilyContainer::GetInstance()->GetCounterFamily(count_name, "");
    if (!counter_family) {
        return;
    }
    auto& counter = counter_family->Add(sub_labels);
    counter.Increment();            
    
    auto* historgram_family = prometheus::FamilyContainer::GetInstance()->GetHistogramFamily(histogram_name, "");
    if (!historgram_family) {
        return;
    }
    auto& historgram = historgram_family->Add(sub_labels,
        prometheus::Histogram::BucketBoundaries{
            FLAGS_hist_latency_p1, 
            FLAGS_hist_latency_p2, 
            FLAGS_hist_latency_p3, 
            FLAGS_hist_latency_p4, 
            FLAGS_hist_latency_p5, 
            FLAGS_hist_latency_p6});//us
    historgram.Observe(latency_us);   
}

static void ReportPromethus(Controller* cntl, 
                            const std::string& name, 
                            const std::map<std::string, std::string>& common_labels,
                            int64_t latency_us) {
                                
    std::map<std::string, std::string> channel_labels(common_labels);
    if (FLAGS_enable_collect_channel) {
        std::string channel_name;
        GetHeader(cntl, "Channel-Name", channel_name);
        channel_labels.insert(std::make_pair("channel", channel_name));
    }
    if (FLAGS_enable_collect_bucket) {
        std::string bucket;
        GetHeader(cntl, "Bucket-Id", bucket);
        channel_labels.insert(std::make_pair("bucket", bucket));
    }
    string count_name(name + kDelimiter + GetUnderscoredModuleName() + kDelimiter + "counter");
    string histogram_name(name + kDelimiter + GetUnderscoredModuleName() + kDelimiter + "histogram");
    ReportPromethusHelper(count_name, histogram_name, channel_labels, latency_us); 	
    
    if (FLAGS_enable_collect_metric_expid) {
        std::vector<std::string> expids;
        GetExpId(cntl, &expids);
        for (auto& expid : expids) {
            std::map<std::string, std::string> exp_labels(common_labels);
            exp_labels.insert(std::make_pair("expid", expid));
    
            ReportPromethusHelper(count_name + kDelimiter + "metric", 
                                  histogram_name + kDelimiter + "metric", 
                                  exp_labels, latency_us);
        }	  
    }
}

BossGuard::BossGuard() {

}

// Channel 用来获取远端的 ip 地址
BossGuard::BossGuard(
    Controller* cntl, 
    Channel* channel, 
    const google::protobuf::Message* request, 
    const int64_t begin_time_us) :
    remote_name_(""), 
    remote_uri_(""), 
    begin_time_us_(begin_time_us), 
    enable_(true) {    
    if (channel) {
        if (!channel->_ns_url.empty()) {
            remote_name_ = channel->_ns_url;
        } else {
            int ret = endpoint2hostname(channel->_server_address, &remote_name_);
            if (0 != ret) {
                //LOG(ERROR)  << "srv:" << channel->_server_address << " endpoint2hostname fail,ret:" << ret;
            }
        }
    }

    if (cntl->request_protocol() == brpc::PROTOCOL_HTTP) {
        ParseHttpUri(cntl, &host_uri_, &remote_uri_);
    } else if (cntl->request_protocol() == brpc::PROTOCOL_REDIS) {
        host_uri_ = "redis-client";
        // redis 区分出命令
        const RedisRequest* redis = static_cast<const RedisRequest*>(request);
        if (redis != NULL) {
            ParseRedisCommand(*redis);
        }
    } else if (cntl->request_protocol() == brpc::PROTOCOL_WUP) {
        host_uri_ = "wup-client";
        const WupRequest* wup = static_cast<const WupRequest*>(request);
        if (wup != NULL) {
            ParseWupCommand(*wup);
        }
    } else {//TODO remote_uri
        host_uri_ = "rpc-client";
    }
}

void BossGuard::ParseWupCommand(const WupRequest& req) {
    const std::string& servant = req.GetServantName();
    size_t pos = servant.find_last_of('.');
    if (pos != std::string::npos) {
        remote_name_ = servant.substr(0, pos);
        remote_uri_  = servant.substr(pos + 1);
    } else {
        remote_uri_ = (!servant.empty()) ? servant : "default";
    }
}

void BossGuard::ParseRedisCommand(const RedisRequest& req) {
    butil::IOBuf cp = req._buf;
    butil::IOBuf seg;
    int i = 0;
    while (cp.cut_until(&seg, "\r\n") == 0) {
        if (i == 2) {
            remote_uri_ = seg.to_string();
        }
        seg.clear();
        i++;
    }
}

void BossGuard::ParseHttpUri(Controller* cntl, std::string* host_uri, std::string* remote_uri) {
    host_uri->clear();
    remote_uri->clear();

    if (!FLAGS_prometheus_enable && FLAGS_mcall_with_bucket) {
        const std::string* bucket = cntl->http_request().GetHeader("Bucket-Id");
        if (bucket != NULL) {
            host_uri->append("[" + *bucket + "]:");
            remote_uri->append("[" + *bucket + "]:");
        }
    }

    const std::string* hostpath = cntl->http_request().GetHeader("Host-Path"); //TODO what is Host-Path???
    if (hostpath != NULL) {
        host_uri->append(*hostpath);
    } else {
        host_uri->append("http-client");
    }

    remote_uri->append(cntl->http_request().uri().path());
    if (remote_name_.find("logcoloring") != std::string::npos) {
        remote_uri->assign(FLAGS_module_name);//TODO
        enable_ = false;
    }
}

void BossGuard::Mcall(Controller* cntl) {
    if (!enable_) {
        return ;
    }
    
    int code = cntl->ErrorCode();
    if (code == EHTTP) {
        code = cntl->http_response().status_code();
    }

    if (cntl->method()) {
        if (remote_name_.empty()) {
             (remote_name_ = cntl->method()->service() ?  cntl->method()->service()->full_name() : remote_name_);
        }
        if (remote_uri_.empty()) {
            remote_uri_ = cntl->method()->full_name();
        }
    }

    int64_t latency_us = butil::gettimeofday_us() - begin_time_us_;
    std::string remote_ip(butil::ip2str(cntl->remote_side().ip).c_str());
    if (FLAGS_mcall_enable) {
        // 1: FLAGS_module_name, 主调模块的 name
        // 2: host_uri_, 主调的接口, 主调模块上报所在的接口
        // 3: remote_name_, 被调模块的 name
        // 4: remote_uri_, 被调的接口
        // 5: FLAGS_mcall_project_id, 模调 id
        // 6: latency_us, 耗时
        // 7: code, 返回码
        // 8: remote_ip, 被调 ip
        int ret = BossAPI::CModCacheApi::instance()->MCReport(FLAGS_module_name, host_uri_,
                                                              remote_name_, remote_uri_,
                                                              FLAGS_mcall_project_id,
                                                              latency_us, code, 
                                                              remote_ip, 0, 1);
        if (ret != 0) {
            if (FLAGS_mcall_verbose || (ret != 12 && ret != 10)) {
                LOG(ERROR) << "mcall error: " << ret;
            }
        }   
    }

    if (FLAGS_prometheus_enable) {
        if (EnablePrometheusMetrics(remote_uri_)) {
            std::map<std::string, std::string> labels({
                {"code", butil::IntToString(code)}, 
                {"remote_name", remote_name_}});
            if(FLAGS_enable_collect_remote_uri) {
                labels.insert(std::make_pair("remote_uri", remote_uri_));
            }
            ReportPromethus(cntl, "calling", labels, latency_us);	
        }         
    }

    VLOG(10) << "client report client module:"<< FLAGS_module_name
            << " uri:" << host_uri_
            << " server module:" << remote_name_
            << " ip:" << remote_ip
            << " uri:" << remote_uri_
            << " mcall_id:" << FLAGS_mcall_project_id
            << " cost:" << latency_us
            << " ErrorCode :" << code;

#if BOSS_TEST
    boss_call << 1;
    LOG(INFO) << "boss_call add";
#endif
}

BossGuard::~BossGuard() {
}

BossHostGuard::BossHostGuard(Controller* in): cntl_(in),
    code_(0) {
    code_ = cntl_->ErrorCode();

    if (cntl_->request_protocol() == brpc::PROTOCOL_HTTP) {
        ParseHTTPUri();

        // 待讨论具体因为什么原因失败
        // if (code_ != 0) {
        //     code_ = cntl_->http_response().status_code();
        //     LOG(INFO) << "http code: " << code_;
        // }
    } else if (cntl_->request_protocol() == brpc::PROTOCOL_BAIDU_STD) {
        if (cntl_->method() != NULL) {
            host_uri_ = cntl_->method()->full_name();
        }
    }

    remote_ip_ = butil::ip2str(cntl_->remote_side().ip).c_str();
}

void BossHostGuard::ParseHTTPUri() {
    host_uri_.clear();

    if (!FLAGS_prometheus_enable && FLAGS_mcall_with_bucket) {
        const std::string* bucket = cntl_->http_request().GetHeader("Bucket-Id");
        if (bucket != NULL) {
            host_uri_.append("[" + *bucket + "]:");
        }
    }

    host_uri_.append(cntl_->http_request().uri().path());
}

void BossHostGuard::Mcall() {
    if (!FLAGS_mcall_enable && !FLAGS_prometheus_enable) {
        return ;
    }
    int64_t latency_us = cntl_->server_rpc_latency_us();
    if (FLAGS_mcall_enable) {
        // 1: FLAGS_module_name, 被调模块, 上报接口所在模块
        // 2: host_uri_, 被调模块接口
        // 3: FLAGS_mcall_project_id, 模调 id
        // 4: latency_us: 耗时
        // 5: code_, 返回码
        // 6: remote_ip_, 远端 ip
        int ret = BossAPI::CModCacheApi::instance()->MCReport("", "", 
                                                              FLAGS_module_name, host_uri_, 
                                                              FLAGS_mcall_project_id,
                                                              latency_us, code_, 
                                                              remote_ip_, 0, 1);
        if (ret != 0) {
            if (FLAGS_mcall_verbose || (ret != 12 && ret != 10)) {
                LOG(ERROR) << "mcall error: " << ret;
            }
        }
    }
    
    if (FLAGS_prometheus_enable) {  
        if (cntl_->method() && cntl_->method()->service()) {
            std::string local_name = cntl_->method()->service()->full_name();
            const auto* sp = cntl_->server()->FindServicePropertyByFullName(local_name);
            if (sp && !sp->is_builtin_service) {
                std::map<std::string, std::string>  labels({
                    {"code", butil::IntToString(code_)}});
                if (FLAGS_enable_collect_local_name) {  
                    labels.insert(std::make_pair("local_name", local_name));
                }
                if (FLAGS_enable_collect_local_uri) {  
                    labels.insert(std::make_pair("local_uri", host_uri_));
                }
                ReportPromethus(cntl_, "called", labels, latency_us);
            }
        }
    }   	 

    VLOG(10) << "server report server module:"<< FLAGS_module_name
            << " ip:" << remote_ip_
            << " uri:" << host_uri_
            << " mcall_id:" << FLAGS_mcall_project_id
            << " cost:" << latency_us
            << " ErrorCode:" << code_;

#if BOSS_TEST
    boss_host_call << 1;
    LOG(INFO) << "boss_host_call add";
#endif
}

BossHostGuard::~BossHostGuard() {
    Mcall();
}

}
