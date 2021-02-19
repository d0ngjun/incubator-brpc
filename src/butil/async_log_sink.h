#ifndef ASYNC_LOG_SINK_H
#define ASYNC_LOG_SINK_H

#include "butil/macros.h"
#include "butil/logging.h"
#include "butil/log_thread.h"
#include "butil/memory/singleton.h"

namespace logging {

class AsyncLogSink : public logging::LogSink {
public:
    static AsyncLogSink* GetInstance() {
        return Singleton<AsyncLogSink>::get();
    }
    
    virtual bool OnLogMessage(int severity, const char* file, int line,
        const butil::StringPiece& log_content) override;
private:
    AsyncLogSink() = default;
    friend struct DefaultSingletonTraits<AsyncLogSink>;
    LogThread log_thread_;
    virtual ~AsyncLogSink();
    DISALLOW_COPY_AND_ASSIGN(AsyncLogSink);
};

}

#endif
