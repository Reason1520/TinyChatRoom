#ifndef _STATUSGRPCCLIENT_H_
#define _STATUSGRPCCLIENT_H_

#include <grpcpp/grpcpp.h>
#include <memory>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

/******************************************************************************
 * @file       statusgrpcclient.h
 * @brief      状态服务的gRPC客户端类，用于获取聊天服务器ip信息和token信息反馈给客户端
 *
 * @author     lueying
 * @date       2026/1/3
 * @history
 *****************************************************************************/

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

class StatusConPool;

class StatusGrpcClient :public Singleton<StatusGrpcClient> {
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient();
    // 获取聊天服务器信息
    GetChatServerRsp getChatServer(int uid);

private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> pool_;
};

#endif // _STATUSGRPCCLIENT_H_