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

#ifndef BRPC_POLICY_TRPC_PROTOCOL_H
#define BRPC_POLICY_TRPC_PROTOCOL_H

#include "brpc/protocol.h"
#include "butil/iobuf.h"                         // butil::IOBuf
#include "trpc.pb.h"

namespace brpc {
namespace policy {
struct TrpcFixedHeader {
  static const uint32_t TRPC_PROTO_PREFIX_SPACE             = 16;
  static const uint32_t TRPC_PROTO_MAGIC_SPACE              = 2;
  static const uint32_t TRPC_PROTO_DATAFRAME_TYPE_SPACE     = 1;
  static const uint32_t TRPC_PROTO_DATAFRAME_STATE_SPACE    = 1;
  static const uint32_t TRPC_PROTO_DATAFRAME_SIZE_SPACE     = 4;
  static const uint32_t TRPC_PROTO_HEADER_SIZE_SPACE        = 2;
  static const uint32_t TRPC_PROTO_STREAM_ID_SPACE          = 2;
  static const uint32_t TRPC_PROTO_REVERSED_SPACE           = 4;

  uint16_t                  magic_value       = trpc::TrpcMagic::TRPC_MAGIC_VALUE;
  uint8_t                   data_frame_type   = 0;
  uint8_t                   data_frame_state  = 0;
  uint32_t                  data_frame_size   = 0;
  uint16_t                  pb_header_size    = 0;
  uint16_t                  stream_id         = 0;
  char                      reversed[4]       = {0};
};

struct TrpcRequestProtocol {
  TrpcFixedHeader           fixed_header;
  trpc::RequestProtocol     req_header;
  butil::IOBuf*             req_body;
};

struct TrpcResponseProtocol {
  TrpcFixedHeader           fixed_header;
  trpc::ResponseProtocol    rsp_header;
  butil::IOBuf*             rsp_body;
};

// Parse binary format of trpc
ParseResult ParseTrpcMessage(butil::IOBuf* source, Socket *socket, bool read_eof,
                            const void *arg);

// Actions to a (client) request in baidu_std format
void ProcessTrpcRequest(InputMessageBase* msg);

// Actions to a (server) response in trpc format.
void ProcessTrpcResponse(InputMessageBase* msg);

// Pack `req_body' to `method' into `buf'.
void PackTrpcRequest(butil::IOBuf* buf,
                    SocketMessage**,
                    uint64_t correlation_id,
                    const google::protobuf::MethodDescriptor* method,
                    Controller* controller,
                    const butil::IOBuf& req_body,
                    const Authenticator*);

}  // namespace policy
} // namespace brpc

#endif  // BRPC_POLICY_TRPC_PROTOCOL_H
