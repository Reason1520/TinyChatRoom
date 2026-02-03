#pragma once
#include "const.h"
#include "singleton.h"
#include "configmgr.h"
#include <grpcpp/grpcpp.h> 
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <queue>
#include "const.h"
#include "data.h"

/******************************************************************************
 * @file       chatgrpcclient.h
 * @brief      grpc客户端类，用于chatserver之间的通信
 *
 * @author     lueying
 * @date       2026/1/29
 * @history
 *****************************************************************************/

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::ChatService;

class ChatConPool {
public:
	ChatConPool(size_t poolSize, std::string host, std::string port)
		: poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
		for (size_t i = 0; i < poolSize_; ++i) {

			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());

			connections_.push(ChatService::NewStub(channel));
		}
	}

	~ChatConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	std::unique_ptr<ChatService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] {
			if (b_stop_) {
				return true;
			}
			return !connections_.empty();
			});
		// 如果停止则直接返回空指针
		if (b_stop_) {
			return nullptr;
		}
		auto context = std::move(connections_.front());
		connections_.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<ChatService::Stub> context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_) {
			return;
		}
		connections_.push(std::move(context));
		cond_.notify_one();
	}

	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}

private:
	std::atomic<bool> b_stop_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<ChatService::Stub> > connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};

class ChatGrpcClient :public Singleton<ChatGrpcClient>
{
	friend class Singleton<ChatGrpcClient>;
public:
	~ChatGrpcClient() {

	}
	// 通知添加好友
	AddFriendRsp notifyAddFriend(std::string server_ip, const AddFriendReq& req);
	// 通知好友申请已通过
	AuthFriendRsp notifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
	// 获取用户基础信息
	bool getBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	// 通知有聊天消息
	TextChatMsgRsp notifyTextChatMsg(std::string server_ip,
		const TextChatMsgReq& req, const Json::Value& rtvalue);
private:
	ChatGrpcClient();
	std::unordered_map<std::string, std::unique_ptr<ChatConPool>> pools_;
};





