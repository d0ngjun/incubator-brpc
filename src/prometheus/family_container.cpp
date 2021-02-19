#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <signal.h>
#include <utility>
#include <gflags/gflags.h>
#include <gflags/gflags_declare.h>
#include <brpc/channel.h>
#include <brpc/controller.h>
#include <brpc/boss_guard.h>

#include "butil/memory/singleton.h"
#include "butil/strings/string_number_conversions.h"
#include "prometheus/text_serializer.h"

#include "family_container.h"

DEFINE_string(crash_report_2_prometheus_product, "Recommend", "prometheus_product");
DEFINE_string(crash_report_2_prometheus_server, "onss://pushgateway.webdev.com", "prometheus_server");
DEFINE_string(crash_report_2_prometheus_lb, "la", "prometheus_lb");
DEFINE_bool(crash_report_2_prometheus_flag, true, "crash_report_2_prometheus_flag");
DEFINE_int32(crash_report_2_prometheus_timeout_ms, 1000, "crash_report_2_prometheus_timeout_ms");

namespace prometheus {

FamilyContainer* FamilyContainer::GetInstance() {
    return Singleton<FamilyContainer>::get();
}

Family<Counter>* FamilyContainer::GetCounterFamily(const std::string& name, const std::string& help) const {
    if (!_registry) {        
        CHECK(_registry) << "_registry is null";
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(_counter_mutex);
        auto it = _counter_family.find(name);
        if (it != _counter_family.end()) {
            return (it->second);
        }
    }
    auto& counter_family = BuildCounter()
                                .Name(name)
                                .Help(help)
                                .Register(*_registry);
    {
        std::lock_guard<std::mutex> lock{_counter_mutex};
        _counter_family.insert(std::make_pair(name, &counter_family));
    }
    return &counter_family;
}

Family<Summary>* FamilyContainer::GetSummaryFamily(const std::string& name, const std::string& help) const {
    if (!_registry) {        
        CHECK(_registry) << "_registry is null";
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(_summary_mutex);
        auto it = _summary_family.find(name);
        if (it != _summary_family.end()) {
            return (it->second);
        }
    }
    auto& summary_family = BuildSummary()
                                .Name(name)
                                .Help(help)
                                .Register(*_registry);
    {
        std::lock_guard<std::mutex> lock(_summary_mutex);
        _summary_family.insert(std::make_pair(name, &summary_family));
    }
    return &summary_family;
}

Family<Histogram>* FamilyContainer::GetHistogramFamily(const std::string& name, const std::string& help) const {
    if (!_registry) {        
        CHECK(_registry) << "_registry is null";
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(_histogram_mutex);
        auto it = _histogram_family.find(name);
        if (it != _histogram_family.end()) {
            return (it->second);
        }
    }
    auto& histogram_family = BuildHistogram()
                                .Name(name)
                                .Help(help)
                                .Register(*_registry);
    {
        std::lock_guard<std::mutex> lock(_histogram_mutex);
        _histogram_family.insert(std::make_pair(name, &histogram_family));
    }
    return &histogram_family;
}

std::string get_local_ip(void) {
    std::string local_ip;
    struct ifaddrs * ifAddrStruct = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct!= NULL) {

        if (ifAddrStruct->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            //printf("%s IP Address %s/n", ifAddrStruct->ifa_name, addressBuffer); 
            if (strcmp(ifAddrStruct->ifa_name, "eth1") == 0) {
                local_ip = addressBuffer;
                return local_ip;
            }
        } else if (ifAddrStruct->ifa_addr->sa_family == AF_INET6) { // check it is IP6 
            // is a valid IP6 Address
            /*
            tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s/n", ifAddrStruct->ifa_name, addressBuffer); 
            */
        } 
            
        ifAddrStruct = ifAddrStruct->ifa_next;
    }
    return local_ip;
}

void ReportCrashSignalHandler(int sig, siginfo_t* info, void* void_context) {
    brpc::ChannelOptions options;
    options.protocol = brpc::PROTOCOL_HTTP;
    options.timeout_ms = FLAGS_crash_report_2_prometheus_timeout_ms;
    brpc::Channel channel;
    if (channel.Init(FLAGS_crash_report_2_prometheus_server.c_str(), FLAGS_crash_report_2_prometheus_lb.c_str(), &options) != 0) {
        LOG(ERROR) << "sig:" << sig << ",Fail to initialize channel" 
            << ",naming_service_url:" << FLAGS_crash_report_2_prometheus_server.c_str() 
            << ",load_balancer_nam:" << FLAGS_crash_report_2_prometheus_lb.c_str();
        return;
    }

    std::string gateway_uri = "/metrics/job/"; //refer:https://prometheus.io/docs/concepts/jobs_instances/
    gateway_uri += brpc::GetUnderscoredModuleName();
    brpc::Controller cntl;
    cntl.http_request().uri() = gateway_uri;
    cntl.http_request().set_method(brpc::HTTP_METHOD_POST);

    std::string ip_self = get_local_ip();
    const std::map<std::string, std::string> labels({
        {"product", FLAGS_crash_report_2_prometheus_product.c_str()},
        {"instance", ip_self},
        {"signal", butil::IntToString(sig)}});
    std::string counter_name = "coredump_";
    counter_name += brpc::GetUnderscoredModuleName();
    
    std::unique_ptr<prometheus::FamilyContainer> fc(new prometheus::FamilyContainer());
    auto* counter_family = fc->GetCounterFamily(counter_name, ""); //TODO name and help
    if (counter_family) {
        auto& counter = counter_family->Add(labels);
        counter.Increment();    
    
        const auto serializer = TextSerializer{};
        auto metrics = fc->GetRegistry()->Collect();
        
        cntl.request_attachment().append(serializer.Serialize(metrics));
    }

    channel.CallMethod(NULL, &cntl, NULL, NULL, NULL/*done*/);
    LOG(INFO) << "sig:" << sig
        << ",uri:" << cntl.http_request().uri()
        << ",err_code:" << cntl.ErrorCode() 
        << ",err_txt:" << cntl.ErrorText() 
        << ",cost:"  << cntl.latency_us()/1000
        << "ms,peer:" << cntl.remote_side()
        << ",status:" << cntl.http_response().status_code()
        << ",req:" << cntl.request_attachment().to_string()
        << ",rsp:" << cntl.response_attachment().to_string();
}

class ReportCrash {
public:
    ReportCrash() {
        if (FLAGS_crash_report_2_prometheus_flag) {
            EnableReportWhenCrash();
        }
    }
    ~ReportCrash() {
    }
private:

    bool EnableReportWhenCrash() {
        struct sigaction action;
        memset(&action, 0, sizeof(action));
        action.sa_flags = SA_RESETHAND | SA_SIGINFO;
        action.sa_sigaction = &ReportCrashSignalHandler;
        sigemptyset(&action.sa_mask);
  
        bool success = false;
        success &= (sigaction(SIGILL, &action, NULL) == 0);//4
        success &= (sigaction(SIGABRT, &action, NULL) == 0);//6
        success &= (sigaction(SIGFPE, &action, NULL) == 0);//8
        success &= (sigaction(SIGBUS, &action, NULL) == 0);//7
        success &= (sigaction(SIGSEGV, &action, NULL) == 0);//11
        //success &= (sigaction(SIGTERM, &action, NULL) == 0);
        return success;
    }

private:
};

static ReportCrash rc;
}
