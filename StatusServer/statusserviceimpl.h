#ifndef STATUSSERVICEIMPL_H_
#define STATUSSERVICEIMPL_H_

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"

/******************************************************************************
 * @file       statusserviceimpl.h
 * @brief      状态服务类，为客户端分配一个可用的 聊天服务器
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

// 记录chat server的信息
class  ChatServer {
public:
	ChatServer() :host(""), port(""), name(""), con_count(0) {}
	ChatServer(const ChatServer& cs) :host(cs.host), port(cs.port), name(cs.name), con_count(cs.con_count) {}
	ChatServer& operator=(const ChatServer& cs) {
		if (&cs == this) {
			return *this;
		}

		host = cs.host;
		name = cs.name;
		port = cs.port;
		con_count = cs.con_count;
		return *this;
	}
	std::string host;
	std::string port;
	std::string name;
	int con_count;
};

class StatusServiceImpl final : public StatusService::Service {
public:
    StatusServiceImpl();
    Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
        GetChatServerRsp* reply) override;
	Status Login(ServerContext* context, const LoginReq* request, LoginRsp* reply) override;

private:
	// 存在内存中的数据
	std::unordered_map<std::string, ChatServer> servers_;
    std::mutex servers_mutex_;
	std::unordered_map<int, std::string> tokens_;
	std::mutex tokens_mutex_;
	// 获取最小连接数的chatserver名字
	ChatServer getAvailableServer();
	// 将token存入redis中
	void insertToken(int uid, std::string token);
};


#endif // STATUSSERVICEIMPL_H_