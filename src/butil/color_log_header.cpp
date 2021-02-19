#include "butil/color_log_header.h"
#include "butil/logging.h"
#include "bthread/task_group.h"
#include "bthread/bthread.h"

namespace bthread {
    extern __thread bthread::LocalStorage tls_bls;
}

namespace logging {
ColorLogHeader::ColorLogHeader() {
    Init();
}

void ColorLogHeader::InitFromeHttpHeader(const brpc::HttpHeader& req_header) {
    Init();
    const std::string* http_header = nullptr;
    if ((http_header = req_header.GetHeader("Dev-Id")) != nullptr && !http_header->empty()) {
        devid_ = *http_header;
        devid_iobuf_ = devid_;
    } 

    if ((http_header = req_header.GetHeader("Wx-Open-Id")) != nullptr && !http_header->empty()) {
        wxopenid_ = *http_header;
        wxopenid_iobuf_ = wxopenid_;
    } 

    if ((http_header = req_header.GetHeader("Uin")) != nullptr && !http_header->empty()) {
        uin_ = strtoull(http_header->c_str(), nullptr, 10);
    } 

    if ((http_header = req_header.GetHeader("Bucket-Id")) != nullptr && !http_header->empty()) {
        bucketid_ = atoi(http_header->c_str());
    } 

    if ((http_header = req_header.GetHeader("Log-Status")) != nullptr && !http_header->empty()) {
        if (http_header->compare("coloring") == 0) {
            log_coloring_ = true;
        }
    }

    if ((http_header = req_header.GetHeader("Log-Report")) != nullptr && !http_header->empty()) {
        if (http_header->compare("report") == 0) {
            log_report_ = true;
        }
    }

    if ((http_header = req_header.GetHeader("Trace-Id")) != nullptr && !http_header->empty()) {
        trace_id_ = strtoull(http_header->c_str(), nullptr, 10);
    } 
}

void ColorLogHeader::Init() {
    uin_ = 0;
    trace_id_ = 0;
    devid_ = "";
    wxopenid_ = "";
    bucketid_ = 0;
    log_report_ = false;
    log_coloring_ = false;
}

ColorLogHeader::operator brpc::HttpHeader() const {
    brpc::HttpHeader header;
    if (uin_ != 0) {
        header.SetHeader("Uin", butil::string_printf(
            "%llu", static_cast<unsigned long long>(uin_)));
    }

    if (trace_id_ != 0) {
        header.SetHeader("Trace-Id", butil::string_printf(
            "%llu", static_cast<unsigned long long>(trace_id_)));
    }

    if (!devid_.empty()) {
        header.SetHeader("Dev-Id", devid_);
    }

    if (!wxopenid_.empty()) {
        header.SetHeader("Wx-Open-Id", wxopenid_);
    }

    if (bucketid_ != 0) {
        header.SetHeader("Bucket-Id", butil::string_printf(
            "%llu", static_cast<unsigned long long>(bucketid_)));
    }

    if (log_report_) {
        header.SetHeader("Log-Report", "report");
    }

    if (log_coloring_) {
        header.SetHeader("Log-Status", "coloring");
    }
    return header;
}

butil::ResourceId<ColorLogHeader> ColorLogHeader::id() const {
    return id_;
}

void ColorLogHeader::set_id(butil::ResourceId<ColorLogHeader> id) {
    id_ = id;
}

ColorLogHeader* CreateColorLogHeader() {
    butil::ResourceId<ColorLogHeader> id;
    ColorLogHeader* p = butil::get_resource<ColorLogHeader>(&id);
    if (p == nullptr) {
        LOG(FATAL) << "get color log header failed";
        return p;
    }
    p->set_id(id);
    return p;
}

ColorLogHeader* GetColorLogHeader() {
    if (ColorLogHeader::bls_color_log_header()) {
        butil::ResourceId<ColorLogHeader> id;
        ColorLogHeader* p = butil::get_resource<ColorLogHeader>(&id);
        if (p == nullptr) {
            LOG(FATAL) << "get color log header failed";
            return p;
        }
        *p = *(ColorLogHeader::bls_color_log_header());
        p->set_id(id);
        return p;
    }

    return nullptr;
}

ColorLogHeader* ColorLogHeader::bls_color_log_header () {
    return reinterpret_cast<ColorLogHeader*>(bthread::tls_bls.color_log_header);
}

void ColorLogHeader::set_bls_color_log_header(ColorLogHeader* header) {
    bthread::tls_bls.color_log_header = header;
}

uint64_t ColorLogHeader::uin() const {
    return uin_;
}

uint64_t ColorLogHeader::trace_id() const {
    return trace_id_;
}

const std::string& ColorLogHeader::devid() const {
    return devid_;
}

const std::string& ColorLogHeader::wxopenid() const {
    return wxopenid_;
}

const butil::IOBuf& ColorLogHeader::devid_iobuf() const {
    return devid_iobuf_;
}

const butil::IOBuf& ColorLogHeader::wxopenid_iobuf() const {
    return wxopenid_iobuf_;
}

uint32_t ColorLogHeader::bucketid() const {
    return bucketid_;
}

bool ColorLogHeader::log_report() const {
    return log_report_;
}

bool ColorLogHeader::log_coloring() const {
    return log_coloring_;
}

}