#include "butil/async_log_sink.h"
#include "butil/log_thread.h"

namespace logging {

AsyncLogSink::~AsyncLogSink() {
    log_thread_.Stop();
}

bool AsyncLogSink::OnLogMessage(int severity, const char* file, int line,
    const butil::StringPiece& log_content) {
    std::ostringstream os;
    print_log_prefix(os, severity, file, line);
    os.write(log_content.data(), log_content.size());
    os << '\n';

    log_thread_.Append(os.str());
    return true;
}

}
