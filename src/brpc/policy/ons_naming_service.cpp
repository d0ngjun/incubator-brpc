// Copyright (c) 2018 Tencent.
// seekerzhang

#include <time.h>

#include <vector>

#include "brpc/policy/ons_naming_service.h"

#include <gflags/gflags.h>
#include <nameapi.h>

using std::vector;
using std::map;
using std::make_pair;

DEFINE_int32(onscall_per_one_loop, 5, "the counts abourt call ons interface in one loop in OnsNameService thread");
DEFINE_int32(ons_host_expire, 10, "ons host expire time(minutes)");

namespace brpc {
namespace policy {

OnsNamingService::OnsNamingService():
    _loop_count(FLAGS_onscall_per_one_loop),
    _host_expire(FLAGS_ons_host_expire * 60) {}

int OnsNamingService::GetServers(const char *service_name,
                    std::vector<ServerNode>* servers) {
    servers->clear();

    // 随机调用若干次 ons 服务获取一部分 host
    // 和之前的合并返回给前端
    time_t now = time(NULL);

    map<ServerNode, time_t> tmp_servers;
    ZkHost hosts[512];
    unsigned hosts_max = sizeof(hosts) / sizeof(ZkHost);
    butil::ip_t ip;

    for (int i = 0; i < _loop_count; ++i) {
        unsigned nr = hosts_max;

        int rv = getHostByKeyMulti(service_name, hosts, &nr);
        if (rv != 0) {
            LOG(WARNING) << "getHostByKeyMulti failed: " << rv << ", service_name: " << service_name;
            continue;
        }

        for (unsigned j = 0; j < nr; ++j) {
            if (butil::str2ip(hosts[j].ip, &ip) != 0) {
                LOG(WARNING) << "invalid ip: " << hosts[j].ip << ", service_name: " << service_name;
                continue;
            }

            ServerNode node(ip, hosts[j].port);
            map<ServerNode, time_t>::iterator it = tmp_servers.find(node);
            if (it == tmp_servers.end()) {
                servers->push_back(node);
                tmp_servers.insert(make_pair(node, now));
            }
        }

        // 当nr < hosts_max时，即可认为getHostByKeyMulti获取了所有的hosts
        if (nr > 0 && nr < hosts_max)
            break;
    }

    for (auto it = inner_servers_.begin();
            it != inner_servers_.end();
            ++it) {
        if (now - it->second < _host_expire) { // _host_expire 分钟之前的节点丢弃
            if (tmp_servers.find(it->first) == tmp_servers.end()) {
                servers->push_back(it->first);
                tmp_servers.insert(make_pair(it->first, it->second));
            }
        }
    }
    inner_servers_.swap(tmp_servers);

    return servers->size() == 0;
}

void OnsNamingService::Describe(
    std::ostream& os, const DescribeOptions&) const {
    os << "ons";
    return;
}

NamingService* OnsNamingService::New() const {
    return new OnsNamingService;
}

void OnsNamingService::Destroy() {
    delete this;
}

} // namespace policy
} // namespace brpc
