// Copyright (c) 2014 Baidu, Inc.
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

// Authors: Fu,Benbo (wonderfu@tencent.com)

#include <google/protobuf/descriptor.h>         // MethodDescriptor
#include <google/protobuf/message.h>            // Message
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "butil/logging.h"                       // LOG()
#include "butil/strings/string_split.h"
#include "butil/strings/string_util.h"
#include "butil/time.h"
#include "butil/iobuf.h"                         // butil::IOBuf
#include "butil/raw_pack.h"                      // RawPacker RawUnpacker
#include "brpc/controller.h"                    // Controller
#include "brpc/socket.h"                        // Socket
#include "brpc/server.h"                        // Server
#include "brpc/span.h"
#include "brpc/compress.h"                      // ParseFromCompressedData
#include "brpc/stream_impl.h"
#include "brpc/rpc_dump.h"                      // SampledRequest
#include "brpc/policy/trpc.pb.h"                // 
#include "brpc/policy/baidu_rpc_meta.pb.h"      // 
#include "brpc/policy/trpc_protocol.h"
#include "brpc/policy/most_common_message.h"
#include "brpc/policy/streaming_rpc_protocol.h"
#include "brpc/details/usercode_backup_pool.h"
#include "brpc/details/controller_private_accessor.h"
#include "brpc/details/server_private_accessor.h"

extern "C" {
void bthread_assign_data(void* data);
}

DECLARE_string(module_name);

namespace brpc {
namespace policy {

// Notes:
// 1. Use /service->full_name()/ + method_name to specify the method to call
// 2. *int* type vars are in network byte order

// Pack header into `buf'
inline void PackTrpcFixedHeader(char* fixed_header, int req_header_size, int req_body_size) {
    butil::RawPacker(fixed_header)
        .pack16(trpc::TrpcMagic::TRPC_MAGIC_VALUE)
        .pack8(0)
        .pack8(0)
        .pack32(TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE + req_header_size + req_body_size)
        .pack16(req_header_size)
        .pack16(0)
        .pack("0000", TrpcFixedHeader::TRPC_PROTO_REVERSED_SPACE);
}

template <typename T>
static void SerializeTrpcFixedHeaderAndReqHeader(
    butil::IOBuf* req_buf, const T& req_header, int req_body_size) {
    const int req_header_size = req_header.ByteSize();
    if (req_header_size <= 244) { // most common cases TODO
        char header_and_meta[TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE + req_header_size];
        PackTrpcFixedHeader(header_and_meta, req_header_size, req_body_size);
        ::google::protobuf::io::ArrayOutputStream arr_out(header_and_meta + TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE, req_header_size);
        ::google::protobuf::io::CodedOutputStream coded_out(&arr_out);
        req_header.SerializeWithCachedSizes(&coded_out); // not calling ByteSize again
        CHECK(!coded_out.HadError());
        req_buf->append(header_and_meta, sizeof(header_and_meta));
    } else {
        char header[TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE];
        PackTrpcFixedHeader(header, req_header_size, req_body_size);
        req_buf->append(header, sizeof(header));
        butil::IOBufAsZeroCopyOutputStream buf_stream(req_buf);
        ::google::protobuf::io::CodedOutputStream coded_out(&buf_stream);
        req_header.SerializeWithCachedSizes(&coded_out);
        CHECK(!coded_out.HadError());
    }
}

ParseResult ParseTrpcMessage(butil::IOBuf* source, Socket* socket,
                            bool /*read_eof*/, const void*) {
    char header_buf[TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE];
    const size_t n = source->copy_to(header_buf, sizeof(header_buf));
    
    if (n < TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE) {
        return MakeParseError(PARSE_ERROR_NOT_ENOUGH_DATA);
    }
    
    TrpcFixedHeader tfh;
    butil::RawUnpacker(header_buf)
        .unpack16(tfh.magic_value)
        .unpack8(tfh.data_frame_type)
        .unpack8(tfh.data_frame_state)
        .unpack32(tfh.data_frame_size)
        .unpack16(tfh.pb_header_size)
        .unpack16(tfh.stream_id)
        .unpack(tfh.reversed, TrpcFixedHeader::TRPC_PROTO_REVERSED_SPACE);
  
    if (tfh.magic_value != trpc::TrpcMagic::TRPC_MAGIC_VALUE) {
        return MakeParseError(PARSE_ERROR_TRY_OTHERS);
    }
    // data_frame_size = TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE + pb_header_size + req_body_size;
    if (tfh.data_frame_size - TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE > FLAGS_max_body_size) {
        // We need this log to report the body_size to give users some clues
        // which is not printed in InputMessenger.
        LOG(ERROR) << "body_size=" << tfh.data_frame_size - TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE << " from "
                   << socket->remote_side() << " is too large";
        return MakeParseError(PARSE_ERROR_TOO_BIG_DATA);
    } else if (source->length() < tfh.data_frame_size) {
        return MakeParseError(PARSE_ERROR_NOT_ENOUGH_DATA);
    }
    source->pop_front(TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE);
    MostCommonMessage* msg = MostCommonMessage::Get();
    source->cutn(&msg->meta, tfh.pb_header_size);
    source->cutn(&msg->payload, tfh.data_frame_size - tfh.pb_header_size - TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE);
    return MakeMessage(msg);
}
// Defined in baidu_rpc_protocol.cpp
void EndRunningCallMethodInPool(
    ::google::protobuf::Service* service,
    const ::google::protobuf::MethodDescriptor* method,
    ::google::protobuf::RpcController* controller,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response,
    ::google::protobuf::Closure* done);
    
// Used by UT, can't be static.
void SendTrpcResponse(Controller* cntl, 
                     const google::protobuf::Message* req,
                     const google::protobuf::Message* res,
                     const Server* server,
                     MethodStatus* method_status,
                     int64_t received_us) {
    BossHostGuard boss_host_guard(cntl);

    ControllerPrivateAccessor accessor(cntl);
    Span* span = accessor.span();
    if (span) {
        span->set_start_send_us(butil::cpuwide_time_us());
    }
    cntl->set_end_rpc_time_us(butil::gettimeofday_us());
    Socket* sock = accessor.get_sending_socket();
    std::unique_ptr<Controller, LogErrorTextAndDelete> recycle_cntl(cntl);
    ConcurrencyRemover concurrency_remover(method_status, cntl, received_us);
    std::unique_ptr<const google::protobuf::Message> recycle_req(req);
    std::unique_ptr<const google::protobuf::Message> recycle_res(res);
    
    if (cntl->IsCloseConnection()) {
        sock->SetFailed();
        return;
    }
    bool append_body = false;
    butil::IOBuf res_body;
    // `res' can be NULL here, in which case we don't serialize it
    // If user calls `SetFailed' on Controller, we don't serialize
    // response either
    CompressType type = cntl->response_compress_type();
    if (res != NULL && !cntl->Failed()) {
        if (!res->IsInitialized()) {
            cntl->SetFailed(
                ERESPONSE, "Missing required fields in response: %s", 
                res->InitializationErrorString().c_str());
        } else if (!SerializeAsCompressedData(*res, &res_body, type)) {
            cntl->SetFailed(ERESPONSE, "Fail to serialize response, "
                            "CompressType=%s", CompressTypeToCStr(type));
        } else {
            append_body = true;
        }
    }

    // Don't use res->ByteSize() since it may be compressed
    size_t rsp_size = 0;
    if (append_body) {
        rsp_size = res_body.length();
    }

    int error_code = cntl->ErrorCode();
    if (error_code == -1) {
        // replace general error (-1) with INTERNAL_SERVER_ERROR to make a
        // distinction between server error and client error
        error_code = EINTERNAL;
    }
    trpc::ResponseProtocol rsp_header;
    rsp_header.set_ret(error_code);
    if (!cntl->ErrorText().empty()) {
        // Only set error_msg when it's not empty since protobuf Message
        // always new the string no matter if it's empty or not.
        rsp_header.set_error_msg(cntl->ErrorText());
    }
    rsp_header.set_content_encoding(cntl->response_compress_type());
  
    butil::IOBuf res_buf;
    SerializeTrpcFixedHeaderAndReqHeader(&res_buf, rsp_header, rsp_size);
    if (append_body) {
        res_buf.append(res_body.movable());
    }

    if (span) {
        span->set_response_size(res_buf.size());
    }
    {
        // Have the risk of unlimited pending responses, in which case, tell
        // users to set max_concurrency.
        Socket::WriteOptions wopt;
        wopt.ignore_eovercrowded = true;
        if (sock->Write(&res_buf, &wopt) != 0) {
            const int errcode = errno;
            PLOG_IF(WARNING, errcode != EPIPE) << "Fail to write into " << *sock;
            cntl->SetFailed(errcode, "Fail to write into %s",
                            sock->description().c_str());
            return;
        }
    }

    if (span) {
        // TODO: this is not sent
        span->set_sent_us(butil::cpuwide_time_us());
    }
}

void ProcessTrpcRequest(InputMessageBase* msg_base) {
    const int64_t start_parse_us = butil::cpuwide_time_us();
    DestroyingPtr<MostCommonMessage> msg(static_cast<MostCommonMessage*>(msg_base));
    SocketUniquePtr socket_guard(msg->ReleaseSocket());
    Socket* socket = socket_guard.get();
    const Server* server = static_cast<const Server*>(msg_base->arg());
    ScopedNonServiceError non_service_error(server);

    trpc::RequestProtocol req_header;
    if (!ParsePbFromIOBuf(&req_header, msg->meta)) {
        LOG(WARNING) << "Fail to parse req_header from " << *socket;
        socket->SetFailed(EREQUEST, "Fail to parse req_header from %s",
                          socket->description().c_str());
        return;
    }

    /*
    std::vector<std::string> sf;
    butil::StringSplitter sp(req_header.func(), '/');
    for (; sp; ++sp) {       
        sf.emplace_back(std::string(sp.field(), sp.length()));
    }
    std::string method_name = JoinString(sf, '.');
    */
    std::vector<std::string> sf;
    butil::SplitString(req_header.func(), '/', &sf);
    const std::string & method_name = sf[sf.size() - 1];
    LOG(WARNING) << "req_header.func():" << req_header.func() << ",method_name:" << method_name;

    SampledRequest* sample = AskToBeSampled();
    if (sample) {
        sample->set_service_name(req_header.callee());
        sample->set_method_name(method_name);
        sample->set_protocol_type(PROTOCOL_TRPC);
        sample->request = msg->payload;
        sample->submit(start_parse_us);
    }

    std::unique_ptr<Controller> cntl(new (std::nothrow) Controller);
    if (NULL == cntl.get()) {
        LOG(WARNING) << "Fail to new Controller";
        return;
    }
    cntl->set_begin_rpc_time_us(butil::gettimeofday_us());
    std::unique_ptr<google::protobuf::Message> req;
    std::unique_ptr<google::protobuf::Message> res;

    ServerPrivateAccessor server_accessor(server);
    ControllerPrivateAccessor accessor(cntl.get());
    const bool security_mode = server->options().security_mode() &&
                               socket->user() == server_accessor.acceptor();
    cntl->set_log_id(req_header.request_id());

    cntl->set_request_compress_type(COMPRESS_TYPE_NONE);//TODO
    accessor.set_server(server)
        .set_security_mode(security_mode)
        .set_peer_id(socket->id())
        .set_remote_side(socket->remote_side())
        .set_local_side(socket->local_side())
        .set_auth_context(socket->auth_context())
        .set_request_protocol(PROTOCOL_BAIDU_STD)
        .set_begin_time_us(msg->received_us())
        .move_in_server_receiving_sock(socket_guard);

    // Tag the bthread with this server's key for thread_local_data().
    if (server->thread_local_options().thread_local_data_factory) {
        bthread_assign_data((void*)&server->thread_local_options());
    }

    Span* span = NULL;

    MethodStatus* method_status = NULL;
    do {
        if (!server->IsRunning()) {
            cntl->SetFailed(ELOGOFF, "Server is stopping");
            break;
        }

        if (socket->is_overcrowded()) {
            cntl->SetFailed(EOVERCROWDED, "Connection to %s is overcrowded",
                            butil::endpoint2str(socket->remote_side()).c_str());
            break;
        }
        
        if (!server_accessor.AddConcurrency(cntl.get())) {
            cntl->SetFailed(
                ELIMIT, "Reached server's max_concurrency=%d",
                server->options().max_concurrency);
            break;
        }

        if (FLAGS_usercode_in_pthread && TooManyUserCode()) {
            cntl->SetFailed(ELIMIT, "Too many user code to run when"
                            " -usercode_in_pthread is on");
            break;
        }

        // NOTE(gejun): jprotobuf sends service names without packages. So the
        // name should be changed to full when it's not.
        butil::StringPiece svc_name(req_header.callee());
        if (svc_name.find('.') == butil::StringPiece::npos) {
            const Server::ServiceProperty* sp =
                server_accessor.FindServicePropertyByName(svc_name);
            if (NULL == sp) {
                cntl->SetFailed(ENOSERVICE, "Fail to find service=%s",
                                req_header.callee().c_str());
                break;
            }
            svc_name = sp->service->GetDescriptor()->full_name();
        }
        const Server::MethodProperty* mp =
            server_accessor.FindMethodPropertyByFullName(
                svc_name, method_name);//TODO
        if (NULL == mp) {
            cntl->SetFailed(ENOMETHOD, "Fail to find method=%s/%s",
                            req_header.callee().c_str(),
                            method_name.c_str());
            break;
        } else if (mp->service->GetDescriptor()
                   == BadMethodService::descriptor()) {
            BadMethodRequest breq;
            BadMethodResponse bres;
            breq.set_service_name(req_header.callee());
            mp->service->CallMethod(mp->method, cntl.get(), &breq, &bres, NULL);
            break;
        }
        // Switch to service-specific error.
        non_service_error.release();
        method_status = mp->status;
        if (method_status) {
            int rejected_cc = 0;
            if (!method_status->OnRequested(&rejected_cc)) {
                cntl->SetFailed(ELIMIT, "Rejected by %s's ConcurrencyLimiter, concurrency=%d",
                                mp->method->full_name().c_str(), rejected_cc);
                break;
            }
        }
        google::protobuf::Service* svc = mp->service;
        const google::protobuf::MethodDescriptor* method = mp->method;
        accessor.set_method(method);
        if (span) {
            span->ResetServerSpanName(method->full_name());
        }
        const int reqsize = static_cast<int>(msg->payload.size());
        butil::IOBuf req_buf;
        butil::IOBuf* req_buf_ptr = &msg->payload;

        CompressType req_cmp_type = COMPRESS_TYPE_NONE;//TODO
        req.reset(svc->GetRequestPrototype(method).New());
        if (!ParseFromCompressedData(*req_buf_ptr, req.get(), req_cmp_type)) {
            cntl->SetFailed(EREQUEST, "Fail to parse request message, "
                            "CompressType=%s, request_size=%d", 
                            CompressTypeToCStr(req_cmp_type), reqsize);
            break;
        }
        
        res.reset(svc->GetResponsePrototype(method).New());
        // `socket' will be held until response has been sent
        google::protobuf::Closure* done = ::brpc::NewCallback<
            Controller*, const google::protobuf::Message*,
            const google::protobuf::Message*, const Server*,
            MethodStatus*, int64_t>(
                &SendTrpcResponse, cntl.get(), 
                req.get(), res.get(), server,
                method_status, msg->received_us());

        // optional, just release resourse ASAP
        msg.reset();
        req_buf.clear();

        if (span) {
            span->set_start_callback_us(butil::cpuwide_time_us());
            span->AsParent();
        }
        if (!FLAGS_usercode_in_pthread) {
            return svc->CallMethod(method, cntl.release(), 
                                   req.release(), res.release(), done);
        }
        if (BeginRunningUserCode()) {
            svc->CallMethod(method, cntl.release(), 
                            req.release(), res.release(), done);
            return EndRunningUserCodeInPlace();
        } else {
            return EndRunningCallMethodInPool(
                svc, method, cntl.release(),
                req.release(), res.release(), done);
        }
    } while (false);
  
    // `cntl', `req' and `res' will be deleted inside `SendTrpcResponse'
    // `socket' will be held until response has been sent
    SendTrpcResponse(cntl.release(), 
                    req.release(), res.release(), server,
                    method_status, msg->received_us());
}

void ProcessTrpcResponse(InputMessageBase* msg_base) {
    const int64_t start_parse_us = butil::cpuwide_time_us();
    DestroyingPtr<MostCommonMessage> msg(static_cast<MostCommonMessage*>(msg_base));
    trpc::ResponseProtocol rsp_header;
    if (!ParsePbFromIOBuf(&rsp_header, msg->meta)) {
        LOG(WARNING) << "Fail to parse from response meta";
        return;
    }
    uint64_t cid_value = msg->socket()->correlation_id();
    if (cid_value == 0) {
        LOG(WARNING) << "Fail to find correlation_id from " << *socket;
        return;
    }
    const bthread_id_t cid = { cid_value };
    Controller* cntl = NULL;
    const int rc = bthread_id_lock(cid, (void**)&cntl);
    if (rc != 0) {
        LOG_IF(ERROR, rc != EINVAL && rc != EPERM)
            << "Fail to lock correlation_id=" << cid << ": " << berror(rc);
        return;
    }
    
    ControllerPrivateAccessor accessor(cntl);
    Span* span = accessor.span();
    if (span) {
        span->set_base_real_us(msg->base_real_us());
        span->set_received_us(msg->received_us());
        span->set_response_size(msg->meta.size() + msg->payload.size() + TrpcFixedHeader::TRPC_PROTO_PREFIX_SPACE);
        span->set_start_parse_us(start_parse_us);
    }
    const int saved_error = cntl->ErrorCode();
    do {
        if (rsp_header.ret() != 0) { // TODO, what about func_ret()
            // If ret is unset, default is 0 = success.
            cntl->SetFailed(rsp_header.ret(), "%s", rsp_header.error_msg().c_str());
            break;
        } 
        // Parse response message iff error code from meta is 0
        const int rsp_body_size = msg->payload.length();
        butil::IOBuf* rsp_body_buf_ptr = &msg->payload;

        //const CompressType res_cmp_type = (trpc::TrpcCompressType)rsp_header.content_encoding();//TODO,support compress
        //const CompressType res_cmp_type = trpc::TrpcCompressType::TRPC_DEFAULT_COMPRESS;//TODO,cannot convert 'trpc::TrpcCompressType' to 'const brpc::CompressType' 
        const CompressType res_cmp_type = COMPRESS_TYPE_NONE;
        cntl->set_response_compress_type(res_cmp_type);
        if (cntl->response()) {
            if (!ParseFromCompressedData(
                    *rsp_body_buf_ptr, cntl->response(), res_cmp_type)) {
                cntl->SetFailed(
                    ERESPONSE, "Fail to parse response message, "
                    "CompressType=%s, response_size=%d", 
                    CompressTypeToCStr(res_cmp_type), rsp_body_size);
            }
        } // else silently ignore the response.        
    } while (0);
    // Unlocks correlation_id inside. Revert controller's
    // error code if it version check of `cid' fails
    msg.reset();  // optional, just release resourse ASAP
    accessor.OnResponse(cid, saved_error);
}
// req_body + req_header + fix_header --> req_buf
void PackTrpcRequest(butil::IOBuf* req_buf,
                    SocketMessage**,
                    uint64_t correlation_id,
                    const google::protobuf::MethodDescriptor* method,
                    Controller* cntl,
                    const butil::IOBuf& req_body,
                    const Authenticator*) {

    if (cntl->connection_type() == CONNECTION_TYPE_SINGLE) {
        return cntl->SetFailed(
            EINVAL, "trpc can't work with CONNECTION_TYPE_SINGLE");
    }
    // Store `correlation_id' into Socket since nshead_mcpack protocol
    // doesn't contain this field
    ControllerPrivateAccessor accessor(cntl);
    accessor.get_sending_socket()->set_correlation_id(correlation_id);

    trpc::RequestProtocol req_header; 
    req_header.set_version(trpc::TrpcProtoVersion::TRPC_PROTO_V1);
    req_header.set_call_type(trpc::TrpcCallType::TRPC_UNARY_CALL);
    req_header.set_message_type(trpc::TrpcMessageType::TRPC_DEFAULT);//设置为0.一般类型
    //设置用户透传信息
    //auto trans_info = req_header.mutable_trans_info();
    //trans_info->insert(context->GetReqTransInfo().begin(), context->GetReqTransInfo().end());
    req_header.set_content_type(trpc::TrpcContentEncodeType::TRPC_PROTO_ENCODE);//设置为0. 代表pb类型
    req_header.set_content_encoding(trpc::TrpcCompressType::TRPC_DEFAULT_COMPRESS);//设置为0. 不压素

    std::stringstream func_name;
    if (method) {
        func_name << "/" << method->service()->full_name() << "/" << method->name();
        req_header.set_callee(method->service()->full_name());//TODO,instead ons of service full name
        req_header.set_func(func_name.str());
    } else if (cntl->rpc_dump_meta()) {
        // Replaying. Keep service-name as the one seen by server.
        func_name << "/" << cntl->rpc_dump_meta()->service_name() << "/" << method->name();
        req_header.set_callee(cntl->rpc_dump_meta()->service_name());
        req_header.set_func(func_name.str());
    } else {
        return cntl->SetFailed(ENOMETHOD, "%s.method is NULL", __FUNCTION__);
    }
    if (cntl->has_log_id()) {
        req_header.set_request_id(cntl->log_id());
    }
    req_header.set_caller(FLAGS_module_name);
    req_header.set_timeout(cntl->timeout_ms());

    // Don't use res->ByteSize() since it may be compressed
    const size_t req_size = req_body.length(); 
    SerializeTrpcFixedHeaderAndReqHeader(req_buf, req_header, req_size);//TODO
    req_buf->append(req_body);
}

}  // namespace policy
} // namespace brpc
