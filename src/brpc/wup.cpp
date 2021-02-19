// Copyright (c) 2020 Tencent, Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Authors: Cao,Hongjin (nealcao@tenent.com)

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include <algorithm>
#include <gflags/gflags.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
#include "butil/string_printf.h"
#include "butil/strings/string_number_conversions.h"
#include "butil/macros.h"
#include "brpc/controller.h"
#include "brpc/wup.h"

DECLARE_bool(wup_verbose);

namespace brpc {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_wup_5fbase_2eproto_impl();
void protobuf_AddDesc_wup_5fbase_2eproto();
void protobuf_AssignDesc_wup_5fbase_2eproto();
void protobuf_ShutdownFile_wup_5fbase_2eproto();

namespace {

const ::google::protobuf::Descriptor* WupRequest_descriptor_ = NULL;
const ::google::protobuf::Descriptor* WupResponse_descriptor_ = NULL;

}  // namespace

void protobuf_AssignDesc_wup_5fbase_2eproto() {
    protobuf_AddDesc_wup_5fbase_2eproto();
    const ::google::protobuf::FileDescriptor* file =
        ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
            "proto/wup_base.proto");
    GOOGLE_CHECK(file != NULL);
    WupRequest_descriptor_ = file->message_type(0);
    WupResponse_descriptor_ = file->message_type(1);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
    ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                                       &protobuf_AssignDesc_wup_5fbase_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
    protobuf_AssignDescriptorsOnce();
    ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
        WupRequest_descriptor_, &WupRequest::default_instance());
    ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
        WupResponse_descriptor_, &WupResponse::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_wup_5fbase_2eproto() {
    delete WupRequest::default_instance_;
    delete WupResponse::default_instance_;
}

void protobuf_AddDesc_wup_5fbase_2eproto_impl() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

#if GOOGLE_PROTOBUF_VERSION >= 3002000
    ::google::protobuf::internal::InitProtobufDefaults();
#else
    ::google::protobuf::protobuf_AddDesc_wup_5fbase_2eproto();
#endif
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\024proto/wup_base.proto\022\004brpc\032 google/pro"
    "tobuf/descriptor.proto\"\014\n\nWupRequest\"\r\n\013"
    "WupResponse", 91);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "proto/wup_base.proto", &protobuf_RegisterTypes);
  WupRequest::default_instance_ = new WupRequest();
  WupResponse::default_instance_ = new WupResponse();
  WupRequest::default_instance_->InitAsDefaultInstance();
  WupResponse::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_wup_5fbase_2eproto);
}

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AddDesc_wup_5fbase_2eproto_once);
void protobuf_AddDesc_wup_5fbase_2eproto() {
    ::google::protobuf::GoogleOnceInit(
        &protobuf_AddDesc_wup_5fbase_2eproto_once,
        &protobuf_AddDesc_wup_5fbase_2eproto_impl);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_wup_5fbase_2eproto {
  StaticDescriptorInitializer_wup_5fbase_2eproto() {
    protobuf_AddDesc_wup_5fbase_2eproto();
  }
} static_descriptor_initializer_wup_5fbase_2eproto_;


// ===================================================================

#ifndef _MSC_VER
#endif  // !_MSC_VER

WupRequest::WupRequest()
    : ::google::protobuf::Message() {
    SharedCtor();
}

void WupRequest::InitAsDefaultInstance() {
}

WupRequest::WupRequest(const WupRequest& from)
    : ::google::protobuf::Message() {
    SharedCtor();
    CopyFrom(from);
}

void WupRequest::SharedCtor() { 
    _is_init = false;
    _is_serialized = false;
    _cached_size_ = 0;
}

WupRequest::~WupRequest() {
    SharedDtor();
}

void WupRequest::SharedDtor() {
    if (this != default_instance_) {
    }
}

void WupRequest::SetCachedSize(int size) const {
    GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
    _cached_size_ = size;
    GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* WupRequest::descriptor() {
    protobuf_AssignDescriptorsOnce();
    return WupRequest_descriptor_;
}

const WupRequest& WupRequest::default_instance() {
    if (default_instance_ == NULL) {
        protobuf_AddDesc_wup_5fbase_2eproto();
    }
    return *default_instance_;
}

WupRequest* WupRequest::default_instance_ = NULL;

WupRequest* WupRequest::New() const {
    return new WupRequest;
}

void WupRequest::Clear() {
    _is_init = false;
    _is_serialized = false;
    _buf.clear();
    _request.Clear();
}

bool WupRequest::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream*) {
    LOG(WARNING) << "You're not supposed to parse a WupRequest";
    return true;
}

void WupRequest::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream*) const {
    LOG(WARNING) << "You're not supposed to serialize a WupRequest";
}

::google::protobuf::uint8* WupRequest::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
    return target;
}

int WupRequest::ByteSize() const {
    int total_size = _buf.size();
    GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
    _cached_size_ = total_size;
    GOOGLE_SAFE_CONCURRENT_WRITES_END();
    return total_size;
}

void WupRequest::MergeFrom(const ::google::protobuf::Message& from) {
    LOG(WARNING) << "You're not supposed to merge WupRequest";
}

void WupRequest::MergeFrom(const WupRequest& from) {
    LOG(WARNING) << "You're not supposed to merge WupRequest";
}

void WupRequest::CopyFrom(const ::google::protobuf::Message& from) {
    if (&from == this) return;
    const WupRequest* source =
        ::google::protobuf::internal::dynamic_cast_if_available<const WupRequest*>(&from);
    if (source == NULL) {
        LOG(WARNING) << "You're not supposed to build WupRequest with another type";
    } else {
        CopyFrom(*source);
    }
}

void WupRequest::CopyFrom(const WupRequest& from) {
    if (&from == this) return;
    Clear();
    _buf = from._buf;
    _request = from._request;
    _is_init = from._is_init;
    _is_serialized = from._is_serialized;
}

bool WupRequest::IsInitialized() const {
    return _is_init;
}

void WupRequest::Swap(WupRequest* other) {
    if (other != this) {
        _buf.swap(other->_buf);
        _request.swap(other->_request);
        std::swap(_is_init, other->_is_init);
        std::swap(_is_serialized, other->_is_serialized);
        std::swap(_cached_size_, other->_cached_size_);
    }
}

::google::protobuf::Metadata WupRequest::GetMetadata() const {
    protobuf_AssignDescriptorsOnce();
    ::google::protobuf::Metadata metadata;
    metadata.descriptor = WupRequest_descriptor_;
    metadata.reflection = NULL;
    return metadata;
}

void WupRequest::Serialize() {
    if (!_is_serialized) {
        std::string r;
        _request.encode(r);
        _buf = r;
        _is_serialized = true;
    }
}

bool WupRequest::SerializeTo(butil::IOBuf* buf) const {
    const_cast<WupRequest*>(this)->Serialize();
    *buf = _buf; 
    return true;
}

void WupRequest::Print(std::ostream& os) const {
    const_cast<WupRequest*>(this)->Serialize();
    std::string r = _buf.to_string();
    os << butil::HexEncode(r.data(), r.size()) << "\n";
}

std::ostream& operator<<(std::ostream& os, const WupRequest& r) {
    r.Print(os);
    return os;
}

// ===================================================================

#ifndef _MSC_VER
#endif  // !_MSC_VER

WupResponse::WupResponse()
    : ::google::protobuf::Message() {
    SharedCtor();
}

void WupResponse::InitAsDefaultInstance() {
}

WupResponse::WupResponse(const WupResponse& from)
    : ::google::protobuf::Message() {
    SharedCtor();
    CopyFrom(from);
}

void WupResponse::SharedCtor() {
    _cached_size_ = 0;
    _nreply = 0;
}

WupResponse::~WupResponse() {
    SharedDtor();
}

void WupResponse::SharedDtor() {
    if (this != default_instance_) {
    }
}

void WupResponse::SetCachedSize(int size) const {
    _cached_size_ = size;
}
const ::google::protobuf::Descriptor* WupResponse::descriptor() {
    protobuf_AssignDescriptorsOnce();
    return WupResponse_descriptor_;
}

const WupResponse& WupResponse::default_instance() {
    if (default_instance_ == NULL) {
        protobuf_AddDesc_wup_5fbase_2eproto();
    }
    return *default_instance_;
}

WupResponse* WupResponse::default_instance_ = NULL;

WupResponse* WupResponse::New() const {
    return new WupResponse;
}

void WupResponse::Clear() {
    _reply.Clear();
    _nreply = 0;
    _cached_size_ = 0;
}

bool WupResponse::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream*) {
    LOG(WARNING) << "You're not supposed to parse a WupResponse";
    return true;
}

void WupResponse::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream*) const {
    LOG(WARNING) << "You're not supposed to serialize a WupResponse";
}

::google::protobuf::uint8* WupResponse::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
    return target;
}

int WupResponse::ByteSize() const {
    return _cached_size_;
}

void WupResponse::MergeFrom(const ::google::protobuf::Message& from) {
    LOG(WARNING) << "You're not supposed to merge WupRequest";
}

void WupResponse::MergeFrom(const WupResponse& from) {
    LOG(WARNING) << "You're not supposed to merge WupRequest";
}

void WupResponse::CopyFrom(const ::google::protobuf::Message& from) {
    if (&from == this) return;
    const WupResponse* source =
        ::google::protobuf::internal::dynamic_cast_if_available<const WupResponse*>(&from);
    if (source == NULL) {
        LOG(WARNING) << "You're not supposed to build WupResponse with another type";
    } else {
        CopyFrom(*source);
    }
}

void WupResponse::CopyFrom(const WupResponse& from) {
    if (&from == this) return;
    Clear();
    _reply = from._reply;
    _nreply = from._nreply;
    _cached_size_ = from._cached_size_;
}

bool WupResponse::IsInitialized() const {
    return reply_size() > 0;
}

void WupResponse::Swap(WupResponse* other) {
    if (other != this) {
        _reply.swap(other->_reply);
        std::swap(_nreply, other->_nreply);
        std::swap(_cached_size_, other->_cached_size_);
    }
}

::google::protobuf::Metadata WupResponse::GetMetadata() const {
    protobuf_AssignDescriptorsOnce();
    ::google::protobuf::Metadata metadata;
    metadata.descriptor = WupResponse_descriptor_;
    metadata.reflection = NULL;
    return metadata;
}

ParseError WupResponse::ConsumePartialIOBuf(butil::IOBuf* source) {
    char header_buf[sizeof(wup_t)];
    const size_t n = source->copy_to(header_buf, sizeof(header_buf));
    if (n < sizeof(wup_t)) {
        return PARSE_ERROR_NOT_ENOUGH_DATA;
    }
    const wup_t* wup = (const wup_t *)header_buf;
    uint32_t total_len = ntohl(wup->total_len);
    if ((total_len - sizeof(header_buf)) > FLAGS_max_body_size) {
        return PARSE_ERROR_TOO_BIG_DATA;
    } else if (source->length() < total_len) {
        return PARSE_ERROR_NOT_ENOUGH_DATA;
    }
    std::string s;
    size_t m = source->cutn(&s, total_len);
    if (FLAGS_wup_verbose) {
        LOG(INFO) << "\n[WUP RESPONSE]\n" << butil::HexEncode(s.data(), s.size());
    }
    CHECK_EQ(total_len, m);
    try {
        _reply.decode(s.data(), total_len);
        _nreply += 1;
        _cached_size_ = total_len;
        return PARSE_OK;
    } catch (std::exception& e) {
        LOG(ERROR) << e.what();
        return PARSE_ERROR_ABSOLUTELY_WRONG;
    }
}

void WupResponse::Print(std::ostream& os) const {
    std::string r;
    const_cast<WupResponse*>(this)->_reply.encode(r);
    os << butil::HexEncode(r.data(), r.size()) << "\n";
}

std::ostream& operator<<(std::ostream& os, const WupResponse& response) {
    if (response.reply_size() == 0) {
        return os << "<empty response>";
    } 
    response.Print(os);
    return os;
}
 
} // namespace brpc
