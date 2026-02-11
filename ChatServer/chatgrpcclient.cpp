#include "chatgrpcclient.h"

/******************************************************************************
 * @file       chatgrpcclient.cpp
 * @brief      grpc客户端类实现
 *
 * @author     lueying
 * @date       2026/1/29
 * @history
 *****************************************************************************/

#include "ChatGrpcClient.h"
#include "redismgr.h"
#include "configmgr.h"
#include "usermgr.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "csession.h"
#include "mysqlmgr.h"

ChatGrpcClient::ChatGrpcClient() {
	auto& cfg = ConfigMgr::getInst();
	auto server_list = cfg["PeerServer"]["Servers"];

	std::vector<std::string> words;

	std::stringstream ss(server_list);
	std::string word;

	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	// 为每一个chatserver创建一个连接池
	for (auto& word : words) {
		if (cfg[word]["Name"].empty()) {
			continue;
		}
		pools_[cfg[word]["Name"]] = std::make_unique<ChatConPool>(5, cfg[word]["Host"], cfg[word]["Port"]);
		std::cout << "creat pool for:" << cfg[word]["Name"] << std::endl;
	}

}

// 通知添加好友
AddFriendRsp ChatGrpcClient::notifyAddFriend(std::string server_ip, const AddFriendReq& req) {
	AddFriendRsp rsp;
	Defer defer([&rsp, &req]() {
		rsp.set_error(ErrorCodes::Success);
		rsp.set_applyuid(req.applyuid());
		rsp.set_touid(req.touid());
		});

	auto find_iter = pools_.find(server_ip);
	if (find_iter == pools_.end()) {
		return rsp;
	}

	auto& cfg = ConfigMgr::getInst();
	auto self_name = cfg["SelfServer"]["Name"];
	// 如果在同一个服务器中，直接通知对方有申请消息（二次检查）
	if (server_ip == self_name) {
		// 如果目标用户就在当前服务器
		auto session = UserMgr::getInstance()->getSession(req.touid());
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  rtvalue;
			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["applyuid"] = req.applyuid();
			rtvalue["name"] = req.name();
			rtvalue["desc"] = req.desc();
			std::string return_str = rtvalue.toStyledString();
			session->send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
		}

		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Status status = stub->NotifyAddFriend(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}

// 获取用户基础信息
bool ChatGrpcClient::getBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {
	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::getInstance()->get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		userinfo->uid = root["uid"].asInt();
		userinfo->name = root["name"].asString();
		userinfo->pwd = root["pwd"].asString();
		userinfo->email = root["email"].asString();
		userinfo->nick = root["nick"].asString();
		userinfo->desc = root["desc"].asString();
		userinfo->sex = root["sex"].asInt();
		userinfo->icon = root["icon"].asString();
		std::cout << "user login uid is  " << userinfo->uid << " name  is "
			<< userinfo->name << " pwd is " << userinfo->pwd << " email is " << userinfo->email << std::endl;
	}
	else {
		//redis中没有则查询mysql
		//查询数据库
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlMgr::getInstance()->getUser(uid);
		if (user_info == nullptr) {
			return false;
		}

		userinfo = user_info;

		//将数据库内容写入redis缓存
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pwd"] = userinfo->pwd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		redis_root["nick"] = userinfo->nick;
		redis_root["desc"] = userinfo->desc;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
		RedisMgr::getInstance()->set(base_key, redis_root.toStyledString());
	}

}

// 通知好友申请已通过
AuthFriendRsp ChatGrpcClient::notifyAuthFriend(std::string server_ip, const AuthFriendReq& req) {
	AuthFriendRsp rsp;
	Defer defer([&rsp, &req]() {
		rsp.set_error(ErrorCodes::Success);
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		});

	auto find_iter = pools_.find(server_ip);
	if (find_iter == pools_.end()) {
		return rsp;
	}

	auto& cfg = ConfigMgr::getInst();
	auto self_name = cfg["SelfServer"]["Name"];
	//直接通知对方有认证通过消息
	if (server_ip == self_name) {
		auto session = UserMgr::getInstance()->getSession(req.touid());
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  rtvalue;
			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["fromuid"] = req.fromuid();
			rtvalue["touid"] = req.touid();
			std::string base_key = USER_BASE_INFO + std::to_string(req.touid());
			auto user_info = std::make_shared<UserInfo>();
			bool b_info = getBaseInfo(base_key, req.touid(), user_info);
			if (b_info) {
				rtvalue["name"] = user_info->name;
				rtvalue["nick"] = user_info->nick;
				rtvalue["icon"] = user_info->icon;
				rtvalue["sex"] = user_info->sex;
			}
			else {
				rtvalue["error"] = ErrorCodes::UidInvalid;
			}


			std::string return_str = rtvalue.toStyledString();
			session->send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
		}

		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Status status = stub->NotifyAuthFriend(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}

// 通知有文字信息
TextChatMsgRsp ChatGrpcClient::notifyTextChatMsg(std::string server_name,
	const TextChatMsgReq& req, const Json::Value& rtvalue) {

	TextChatMsgRsp rsp;
	rsp.set_error(ErrorCodes::Success);

	Defer defer([&rsp, &req]() {
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		for (const auto& text_data : req.textmsgs()) {
			TextChatData* new_msg = rsp.add_textmsgs();
			new_msg->set_msg_id(text_data.msg_id());
			new_msg->set_msgcontent(text_data.msgcontent());
		}

		});

	auto find_iter = pools_.find(server_name);
	if (find_iter == pools_.end()) {
		std::cout << "Error: ChatGrpcClient could not find pool for server: " << server_name << std::endl;
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}

// 通知踢掉客户端
KickUserRsp ChatGrpcClient::notifyKickUser(std::string server_ip, const KickUserReq& req) {
	KickUserRsp rsp;
	Defer defer([&rsp, &req]() {
		rsp.set_error(ErrorCodes::Success);
		rsp.set_uid(req.uid());
		});

	auto find_iter = pools_.find(server_ip);
	if (find_iter == pools_.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});
	Status status = stub->NotifyKickUser(&context, req, &rsp);

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}