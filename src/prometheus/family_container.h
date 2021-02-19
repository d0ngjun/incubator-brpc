#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <google/protobuf/service.h>

#include "butil/macros.h"
#include "butil/logging.h"                           // CHECK

#include "prometheus/family.h"
#include "prometheus/counter.h"
#include "prometheus/summary.h"
#include "prometheus/histogram.h"
#include "prometheus/registry.h"

template <typename T> struct DefaultSingletonTraits;

namespace prometheus {

class FamilyContainer {
public:
    static FamilyContainer* GetInstance();

    Family<prometheus::Counter>* GetCounterFamily(const std::string& name, const std::string& help) const;
    Family<prometheus::Summary>* GetSummaryFamily(const std::string& name, const std::string& help) const;
    Family<prometheus::Histogram>* GetHistogramFamily(const std::string& name, const std::string& help) const;

    inline std::shared_ptr<prometheus::Registry> GetRegistry(void) {
        CHECK(_registry) << "_registry is null";
        return _registry;
    }

private:
friend void ReportCrashSignalHandler(int sig, siginfo_t* info, void* void_context);

    FamilyContainer() {
        _registry = std::make_shared<prometheus::Registry>();
        CHECK(_registry) << "_registry is null";
    }

    mutable std::unordered_map<std::string, Family<prometheus::Counter>*> _counter_family;
    mutable std::unordered_map<std::string, Family<prometheus::Summary>*> _summary_family;
    mutable std::unordered_map<std::string, Family<prometheus::Histogram>*> _histogram_family;
    mutable std::mutex _counter_mutex;
    mutable std::mutex _summary_mutex;
    mutable std::mutex _histogram_mutex;

    std::shared_ptr<prometheus::Registry> _registry;

    friend struct DefaultSingletonTraits<FamilyContainer>;

    DISALLOW_COPY_AND_ASSIGN(FamilyContainer);
};

} // prometheus

