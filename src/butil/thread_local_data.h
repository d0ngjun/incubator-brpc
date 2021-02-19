#ifndef THREAD_LOCAL_DATA_H
#define THREAD_LOCAL_DATA_H

#include "brpc/data_factory.h"

namespace brpc {

struct ThreadLocalData {
    uint64_t log_id;
    uint64_t uin;
    uint64_t trace_id;
    std::string devid;
    std::string wxopenid;
    uint32_t bucketid;
    bool log_report;
    bool log_coloring;

    ThreadLocalData() {
        Reset();
    }
    
    void Reset() {
        log_id = 0;
        uin = 0;
        trace_id = 0;
        devid = "";
        wxopenid = "";
        bucketid = 0;
        log_report = false;
        log_coloring = false;
    }
};

class ThreadLocalDataFactory : public brpc::DataFactory {
public:
    void* CreateData() const {
        return new ThreadLocalData;
    }

    void DestroyData(void* d) const {
         delete static_cast<ThreadLocalData*>(d);  
    }
};
}
#endif
