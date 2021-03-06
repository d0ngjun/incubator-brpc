// Copyright (c) 2018 Tencent.
// seekerzhang

#ifndef BRPC_POLICY_ONS_STATEFUL_NAMING_SERVICE_H
#define BRPC_POLICY_ONS_STATEFUL_NAMING_SERVICE_H

#include "brpc/periodic_naming_service.h"

namespace brpc {
namespace policy {

class OnsStatefulNamingService : public PeriodicNamingService {
public:
    OnsStatefulNamingService();
    int GetServers(const char *service_name,
                   std::vector<ServerNode>* servers);
    
    void Describe(std::ostream& os, const DescribeOptions&) const;
    
    NamingService* New() const;
    
    void Destroy();

private:
    std::map<ServerNode, time_t> inner_servers_;
    int _loop_count;
    int _host_expire;
};

} // namespace policy
} // namespace brpc

#endif // BRPC_POLICY_ONS_STATEFUL_NAMING_SERVICE_H

