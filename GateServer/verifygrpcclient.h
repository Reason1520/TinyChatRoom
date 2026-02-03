#ifndef VERIFYGRPCCLIENT_H
#define VERIFYGRPCCLIENT_H

#include <grpcpp/grpcpp.h>
#include <memory>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

/******************************************************************************
 * @file       verifygrpcclient.h
 * @brief      验证服务的gRPC客户端类，用于发起验证码服务端的gRPC调用
 *
 * @author     lueying
 * @date       2025/12/18
 * @history
 *****************************************************************************/

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class RPConPool;
class VerifyGrpcClient :public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;
public:
    // 获取验证码
    GetVerifyRsp getVerifyCode(std::string email);
    ~VerifyGrpcClient();

private:
    VerifyGrpcClient();

    std::unique_ptr<RPConPool> pool_;   // 连接池
};


#endif // VERIFYGRPCCLIENT_H