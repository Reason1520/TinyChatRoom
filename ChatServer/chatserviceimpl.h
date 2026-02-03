#ifndef CHATSERVICEIMPL_H
#define CHATSERVICEIMPL_H

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>
#include "data.h"

/******************************************************************************
 * @file       chserviceimpl.h
 * @brief      grpc服务端类，用于chatserver之间的通信
 *
 * @author     lueying
 * @date       2026/1/29
 * @history
 *****************************************************************************/

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

using message::ChatService;

class ChatServiceImpl final : public ChatService::Service
{
public:
	ChatServiceImpl();
	Status NotifyAddFriend(ServerContext* context, const AddFriendReq* request,
		AddFriendRsp* reply) override;

	Status NotifyAuthFriend(ServerContext* context,
		const AuthFriendReq* request, AuthFriendRsp* response) override;

	Status NotifyTextChatMsg(::grpc::ServerContext* context,
		const TextChatMsgReq* request, TextChatMsgRsp* reply) override;

	bool getBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);

private:
};


#endif // CHATSERVICEIMPL_H