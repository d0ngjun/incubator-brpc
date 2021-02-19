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
//
//
// Authors: Cao,Hongjin (nealcao@tencent.com)

#ifndef BRPC_WUP_H
#define BRPC_WUP_H 

#include <string>
#include <google/protobuf/stubs/common.h>

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
#include "google/protobuf/descriptor.pb.h"

#include "butil/iobuf.h"
#include "butil/strings/string_piece.h"
#include "butil/arena.h"
#include "butil/third_party/taf/jce/wup.h"  // wup::TafUniPacket<>
#include "parse_result.h"

namespace brpc {

struct wup_t {
    uint32_t total_len; // body_len + sizeof(total_len)
};

class WupRequest : public ::google::protobuf::Message {
public:
    WupRequest();
    virtual ~WupRequest();
    WupRequest(const WupRequest& from);
    inline WupRequest& operator=(const WupRequest& from) {
        CopyFrom(from);
        return *this;
    }
    void Swap(WupRequest* other);
    bool SerializeTo(butil::IOBuf* buf) const;
    void SetTafVersion(short v) {
        _request.setTafVersion(v);
    }
    void SetTafPacketType(char v) {
        _request.setTafPacketType(v);
    }
    void SetTafMessageType(int32_t v) {
        _request.setTafMessageType(v);
    }
    void SetTafTimeout(int32_t v) {
        _request.setTafTimeout(v);
    }
    void SetRequestId(int32_t id) {
        _request.setRequestId(id);
    }
    void SetServantName(const std::string& servant) {
        _request.setServantName(servant);
    }
    void SetFuncName(const std::string& func) {
        _request.setFuncName(func);
    }
    template<typename T>
    void Put(const string &name, const T &t) {
        _is_init = true;
        _request.put<T>(name, t);
    }
    const std::string& GetServantName() const {
        return _request.getServantName();
    }

    // Protobuf methods
    WupRequest* New() const;
    void CopyFrom(const ::google::protobuf::Message& from);
    void MergeFrom(const ::google::protobuf::Message& from);
    void CopyFrom(const WupRequest& from);
    void MergeFrom(const WupRequest& from);
    void Clear();
    bool IsInitialized() const;
 
    int ByteSize() const;
    bool MergePartialFromCodedStream(
        ::google::protobuf::io::CodedInputStream* input);
    void SerializeWithCachedSizes(
        ::google::protobuf::io::CodedOutputStream* output) const;
    ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
    int GetCachedSize() const { return _cached_size_; }
    
    static const ::google::protobuf::Descriptor* descriptor();
    static const WupRequest& default_instance();
    ::google::protobuf::Metadata GetMetadata() const;

    void Print(std::ostream& os) const;

private:
    void Serialize();
    void SharedCtor();
    void SharedDtor();
    void SetCachedSize(int size) const;

    wup::TafUniPacket<> _request; 
    butil::IOBuf _buf;
    bool _is_init;
    bool _is_serialized;
    mutable int _cached_size_;

friend void protobuf_AddDesc_wup_5fbase_2eproto_impl();
friend void protobuf_AddDesc_wup_5fbase_2eproto();
friend void protobuf_AssignDesc_wup_5fbase_2eproto();
friend void protobuf_ShutdownFile_wup_5fbase_2eproto();
  
    void InitAsDefaultInstance();
    static WupRequest* default_instance_;
};

class WupResponse : public ::google::protobuf::Message {
public:
    WupResponse();
    virtual ~WupResponse();
    WupResponse(const WupResponse& from);
    inline WupResponse& operator=(const WupResponse& from) {
        CopyFrom(from);
        return *this;
    }
    void Swap(WupResponse* other);
    int reply_size() const { return _nreply; }
    const wup::TafUniPacket<>& reply() const {
        return _reply;
    }
    ParseError ConsumePartialIOBuf(butil::IOBuf* source);
    int GetResultCode() const {
        return _reply.getTafResultCode();
    }
    template<typename T>  
    void Get(const string &name,T &t) {
        _reply.get<T>(name, t);
    }

    // Protobuf methods
    WupResponse* New() const;
    void CopyFrom(const ::google::protobuf::Message& from);
    void MergeFrom(const ::google::protobuf::Message& from);
    void CopyFrom(const WupResponse& from);
    void MergeFrom(const WupResponse& from);
    void Clear();
    bool IsInitialized() const;
  
    int ByteSize() const;
    bool MergePartialFromCodedStream(
        ::google::protobuf::io::CodedInputStream* input);
    void SerializeWithCachedSizes(
        ::google::protobuf::io::CodedOutputStream* output) const;
    ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
    int GetCachedSize() const { return _cached_size_; }

    static const ::google::protobuf::Descriptor* descriptor();
    static const WupResponse& default_instance();
    ::google::protobuf::Metadata GetMetadata() const;
 
    void Print(std::ostream& os) const;

private:
    void SharedCtor();
    void SharedDtor();
    void SetCachedSize(int size) const;

    wup::TafUniPacket<> _reply; 
    int _nreply;
    mutable int _cached_size_;

friend void protobuf_AddDesc_wup_5fbase_2eproto_impl();
friend void protobuf_AddDesc_wup_5fbase_2eproto();
friend void protobuf_AssignDesc_wup_5fbase_2eproto();
friend void protobuf_ShutdownFile_wup_5fbase_2eproto();
  
    void InitAsDefaultInstance();
    static WupResponse* default_instance_;

};

std::ostream& operator<<(std::ostream& os, const WupRequest&);
std::ostream& operator<<(std::ostream& os, const WupResponse&);

} // namespace brpc
#endif // BRPC_WUP_H
