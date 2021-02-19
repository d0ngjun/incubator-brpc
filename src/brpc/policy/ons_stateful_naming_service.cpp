// Copyright (c) 2018 Tencent.
// seekerzhang

#include <time.h>

#include <vector>
#include <string>

#include "brpc/policy/ons_stateful_naming_service.h"
#include "butil/rand_util.h"

#include <gflags/gflags.h>
#include <nameapi.h>

using std::vector;
using std::map;
using std::string;
using std::make_pair;

DEFINE_int32(ons_stateful_call_per_one_loop, 10, "the counts abourt call ons interface in one loop in OnsNameService thread");
DEFINE_int32(ons_stateful_host_expire, 10, "ons host expire time(minutes)");

namespace brpc {
namespace policy {

OnsStatefulNamingService::OnsStatefulNamingService():
    _loop_count(FLAGS_ons_stateful_call_per_one_loop),
    _host_expire(FLAGS_ons_stateful_host_expire * 60) {}

int OnsStatefulNamingService::GetServers(const char *service_name,
                    std::vector<ServerNode>* servers) {
    servers->clear();

    // 随机调用若干次 ons 服务获取一部分 host
    // 和之前的合并返回给前端
    time_t now = time(NULL);

    map<ServerNode, time_t> tmp_servers;
    ZkHost host;

    for (int i = 0; i != _loop_count; ++i) {
        string key = butil::RandBytesAsString(32);
        int rv = getHostByKeyExTimeout(service_name, key.c_str(), &host, 10);
        if (rv != 0) {
            LOG(WARNING) << "getHostByKeyExTimeout failed: " << rv << ", service_name: " << service_name;
            continue;
        }

        butil::ip_t ip;
        if (butil::str2ip(host.ip, &ip) != 0) {
            LOG(WARNING) << "Invalid ip=" << host.ip;
            continue;
        }
        ServerNode node(ip, host.port);
        map<ServerNode, time_t>::iterator it = tmp_servers.find(node);
        if (it == tmp_servers.end()) {
            servers->push_back(node);
            tmp_servers.insert(make_pair(node, now));
        }
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

void OnsStatefulNamingService::Describe(
    std::ostream& os, const DescribeOptions&) const {
    os << "ons_stateful";
    return;
}

NamingService* OnsStatefulNamingService::New() const {
    return new OnsStatefulNamingService;
}

void OnsStatefulNamingService::Destroy() {
    delete this;
}

} // namespace policy
} // namespace brpc

