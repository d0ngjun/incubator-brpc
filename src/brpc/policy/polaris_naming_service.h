#ifndef BRPC_POLICY_POLARIS_NAMING_SERVICE_H
#define BRPC_POLICY_POLARIS_NAMING_SERVICE_H

#include <memory>

#include "brpc/periodic_naming_service.h"
#include "polaris/consumer.h"

namespace brpc {
namespace policy {

class PolarisNamingService : public PeriodicNamingService {
public:
    PolarisNamingService();
    int GetServers(const char *service_names, std::vector<ServerNode>* servers);

    void Describe(std::ostream& os, const DescribeOptions&) const;

    NamingService* New() const;

    void Destroy();

private: 
    std::unique_ptr<polaris::ConsumerApi> _consumer;
};

} // namespace policy
} // namespace brpc

#endif // BRPC_POLICY_POLARIS_NAMING_SERVICE_H
