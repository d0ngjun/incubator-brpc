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

// Authors: Cao,Hongjin (nealcao@tencent.com)

#include <google/protobuf/descriptor.h>         // MethodDescriptor
#include <google/protobuf/message.h>            // Message
#include <gflags/gflags.h>
#include "butil/logging.h"                      // LOG()
#include "butil/time.h"
#include "butil/iobuf.h"                        // butil::IOBuf
#include "brpc/controller.h"                    // Controller
#include "brpc/details/controller_private_accessor.h"
#include "brpc/socket.h"                        // Socket
#include "brpc/server.h"                        // Server
#include "brpc/details/server_private_accessor.h"
#include "brpc/span.h"
#include "brpc/wup.h"
#include "brpc/policy/wup_protocol.h"

DEFINE_bool(wup_verbose, false,
            "[DEBUG] Print wup request/response");

namespace brpc {

namespace policy {

struct InputWupResponse : public InputMessageBase {
    WupResponse response;
    //  @InputMessageBase
    void DestroyImpl() {
        delete this;
    }
};

ParseResult ParseWupMessage(butil::IOBuf* source, Socket* socket,
                            bool /*read_eof*/, const void* /*arg*/) {
    if (source->empty()) {
        return MakeParseError(PARSE_ERROR_NOT_ENOUGH_DATA);
    }
    InputWupResponse* msg = static_cast<InputWupResponse*>(socket->parsing_context());
    if (msg == NULL) {
        msg = new InputWupResponse;
        socket->reset_parsing_context(msg);
    }
    ParseError err = msg->response.ConsumePartialIOBuf(source);
    if (err != PARSE_OK) {
        return MakeParseError(err);
    }
    socket->release_parsing_context();
    return MakeMessage(msg);
}

void ProcessWupResponse(InputMessageBase* msg_base) {
    const int64_t start_parse_us = butil::cpuwide_time_us();
    DestroyingPtr<InputWupResponse> msg(static_cast<InputWupResponse*>(msg_base));
    Socket* socket = msg->socket();

    // Fetch correlation id that we saved before in `PackWupRequest'
    const bthread_id_t cid = { static_cast<uint64_t>(socket->correlation_id()) };
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
       span->set_response_size(msg->response.ByteSize());
       span->set_start_parse_us(start_parse_us);
    }
    const int saved_error = cntl->ErrorCode();
    if (cntl->response() != NULL) {
       if (cntl->response()->GetDescriptor() != WupResponse::descriptor()) {
            cntl->SetFailed(ERESPONSE, "Must be WupResponse");
       } else {
            // We work around ParseFrom of pb which is just a placeholder.
            ((WupResponse*)cntl->response())->Swap(&msg->response);
       }
    } // silently ignore the response.

    // Unlocks correlation_id inside. Revert controller's
    // error code if it version check of `cid' fails
    msg.reset();  // optional, just release resourse ASAP
    accessor.OnResponse(cid, saved_error);
}

void SerializeWupRequest(butil::IOBuf* buf,
                         Controller* cntl,
                         const google::protobuf::Message* request) {
    if (request == NULL)  {
        return cntl->SetFailed(EREQUEST, "request is NULL");
    }
    if (request->GetDescriptor() != WupRequest::descriptor()) {
        return cntl->SetFailed(EREQUEST, "The request is not a WupRequest");
    }
    const WupRequest* wr = (const WupRequest*)request;
    // We work around SerializeTo of pb which is just a placeholder.
    if (!wr->SerializeTo(buf)) {
        return cntl->SetFailed(EREQUEST, "Fail to serialize WupRequest");
    }
    if (FLAGS_wup_verbose) {
      LOG(INFO) << "\n[WUP REQUEST]\n" << *wr;
    }
}

void PackWupRequest(butil::IOBuf* buf,
                    SocketMessage**,
                    uint64_t correlation_id,
                    const google::protobuf::MethodDescriptor*,
                    Controller* cntl,
                    const butil::IOBuf& request,
                    const Authenticator* auth) {
    ControllerPrivateAccessor accessor(cntl);
    if (cntl->connection_type() == CONNECTION_TYPE_SINGLE) {
        return cntl->SetFailed(
                EINVAL, "wup protocol can't work with CONNECTION_TYPE_SINGLE");
    }           
    // Store `correlation_id' into Socket since wup protocol doesn't
    // contain this field
    accessor.get_sending_socket()->set_correlation_id(correlation_id);
    buf->append(request);
}

}  // namespace policy
} // namespace brpc
