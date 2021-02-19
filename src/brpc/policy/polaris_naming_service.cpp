#include <gflags/gflags.h>

#include "brpc/log.h"
#include "brpc/policy/polaris_naming_service.h"
#include "butil/string_splitter.h"

namespace brpc {
namespace policy {

DEFINE_string(polaris_namespace, "Production", "Production,Pre-release,Test,Development,Polaris");
DEFINE_int32(polaris_retry_num, 3, "polaris_server_retry_num");
DEFINE_int32(polaris_timeout_ms, -1, "Timeout for sending polaris request");

static int get_servers_from(const std::string &service_name, polaris::ConsumerApi *consumer, std::vector<ServerNode>* servers)
{
    polaris::ServiceKey service_key = {FLAGS_polaris_namespace, service_name};
    polaris::GetInstancesRequest request(service_key);
    if (FLAGS_polaris_timeout_ms > 0)
        request.SetTimeout(FLAGS_polaris_timeout_ms);

    int retry_num = FLAGS_polaris_retry_num;
    polaris::ReturnCode rc = polaris::kReturnOk;
    polaris::InstancesResponse *response = nullptr;
    butil::ip_t ip;

    do {
        rc = consumer->GetInstances(request, response);
        if (rc != polaris::kReturnOk) {
            LOG(WARNING) << "ConsumerApi::GetInstance failed: " << polaris::ReturnCodeToMsg(rc) << ", service_name: " << service_name;
            continue;
        }

        for (auto &i: response->GetInstances())
            if (butil::str2ip(i.GetHost().c_str(), &ip) == 0)
                servers->emplace_back(ip, i.GetPort());

        delete response;
        response = nullptr;
    } while (rc != polaris::kReturnOk && --retry_num > 0);

    return servers->size() == 0;
}

PolarisNamingService::PolarisNamingService(): _consumer(polaris::ConsumerApi::CreateWithDefaultFile()) {
}

int PolarisNamingService::GetServers(const char *service_names,
    std::vector<ServerNode>* servers) {

    servers->clear();

    for (butil::StringSplitter sp(service_names, ';'); sp; ++sp)
        get_servers_from(std::string(sp.field(), sp.length()), _consumer.get(), servers);

    return servers->size() == 0;
}

void PolarisNamingService::Describe(
    std::ostream& os, const DescribeOptions&) const {
    os << "polaris";
    return;
}

NamingService* PolarisNamingService::New() const {
    return new PolarisNamingService;
}

void PolarisNamingService::Destroy() {
    delete this;
}

} // namespace policy
} // namespace brpc
