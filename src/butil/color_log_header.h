#pragma once
#include <string>
#include <butil/iobuf.h>
#include "brpc/http_header.h"
#include "butil/resource_pool.h"

namespace logging {
class ColorLogHeader {
public:
    ColorLogHeader();
    void InitFromeHttpHeader(const brpc::HttpHeader& req_header);
    operator brpc::HttpHeader() const;
    butil::ResourceId<ColorLogHeader> id() const;
    void set_id(butil::ResourceId<ColorLogHeader> id);
    static ColorLogHeader* bls_color_log_header();
    static void set_bls_color_log_header(ColorLogHeader* header);
    uint64_t uin() const;
    uint64_t trace_id() const;
    const std::string& devid() const;
    const std::string& wxopenid() const;    
    const butil::IOBuf& devid_iobuf() const;
    const butil::IOBuf& wxopenid_iobuf() const;
    uint32_t bucketid() const;
    bool log_report() const;
    bool log_coloring() const;
private:
    void Init();
    uint64_t uin_;
    uint64_t trace_id_;
    std::string devid_;
    std::string wxopenid_;
    butil::IOBuf devid_iobuf_;
    butil::IOBuf wxopenid_iobuf_;
    uint32_t bucketid_;
    bool log_report_;
    bool log_coloring_;
    butil::ResourceId<ColorLogHeader> id_;
};

ColorLogHeader* GetColorLogHeader();
ColorLogHeader* CreateColorLogHeader();
}
