#include "logicsystem.h"
#include "statusgrpcclient.h"
#include "mysqlmgr.h"
#include "configmgr.h"
#include "redismgr.h"
#include "usermgr.h"
#include "const.h"
#include "chatgrpcclient.h"
#include "cserver.h"
#include "utils.h"

/******************************************************************************
 * @file       logicsystem.cpp
 * @brief      逻辑系统的实现
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

LogicSystem::LogicSystem() :b_stop_(false) {
    registerCallBacks();
    worker_thread_ = std::thread(&LogicSystem::dealMsg, this);
}

LogicSystem::~LogicSystem() {
    b_stop_ = true;
    consume_.notify_one();
    worker_thread_.join();
}

void LogicSystem::postMsgToQue(std::shared_ptr <LogicNode> msg) {
	std::unique_lock<std::mutex> unique_lk(mutex_);
	msg_queue_.push(msg);
	//由0变为1则发送通知信号
	if (msg_queue_.size() == 1) {
		unique_lk.unlock();
		consume_.notify_one();
	}
}

void LogicSystem::setServer(std::shared_ptr<CServer> pserver) {
	p_server_ = pserver;
}


void LogicSystem::dealMsg() {
	for (;;) {
		std::unique_lock<std::mutex> unique_lk(mutex_);
		//判断队列为空则用条件变量阻塞等待，并释放锁
		while (msg_queue_.empty() && !b_stop_) {
			consume_.wait(unique_lk);
		}

		//判断是否为关闭状态，把所有逻辑执行完后则退出循环
		if (b_stop_) {
			while (!msg_queue_.empty()) {
				auto msg_node = msg_queue_.front();
				std::cout << "recv_msg id  is " << msg_node->recvnode_->msg_id_ << std::endl;
				auto call_back_iter = fun_callbacks_.find(msg_node->recvnode_->msg_id_);
				if (call_back_iter == fun_callbacks_.end()) {
					msg_queue_.pop();
					continue;
				}
				// 调用该消息的回调函数
				call_back_iter->second(msg_node->session_, msg_node->recvnode_->msg_id_,
					std::string(msg_node->recvnode_->data_, msg_node->recvnode_->cur_len_));
				msg_queue_.pop();
			}
			break;
		}

		//如果没有停服，且说明队列中有数据
		auto msg_node = msg_queue_.front();
		std::cout << "recv_msg id  is " << msg_node->recvnode_->msg_id_ << std::endl;
		auto call_back_iter = fun_callbacks_.find(msg_node->recvnode_->msg_id_);
		if (call_back_iter == fun_callbacks_.end()) {
			msg_queue_.pop();
			std::cout << "msg id [" << msg_node->recvnode_->msg_id_ << "] handler not found" << std::endl;
			continue;
		}
		// std::function 重载了 () 运算符
		call_back_iter->second(msg_node->session_, msg_node->recvnode_->msg_id_,
			std::string(msg_node->recvnode_->data_, msg_node->recvnode_->cur_len_));
		msg_queue_.pop();
	}
}

void LogicSystem::registerCallBacks() {
	// 注册聊天登录回调函数
    fun_callbacks_[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::loginHandler, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 注册搜索好友回调函数
	fun_callbacks_[ID_SEARCH_USER_REQ] = std::bind(&LogicSystem::searchInfo, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	// 注册添加好友回调函数
	fun_callbacks_[ID_ADD_FRIEND_REQ] = std::bind(&LogicSystem::addFriendApply, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	// 注册同意添加好友信息回调函数
	fun_callbacks_[ID_AUTH_FRIEND_REQ] = std::bind(&LogicSystem::authFriendApply, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	// 注册文本聊天消息回调函数
	fun_callbacks_[ID_TEXT_CHAT_MSG_REQ] = std::bind(&LogicSystem::dealChatTextMsg, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	// 注册心跳处理回调函数
	fun_callbacks_[ID_HEART_BEAT_REQ] = std::bind(&LogicSystem::heartBeatHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
	// 注册加载聊天线程回调函数
	fun_callbacks_[ID_LOAD_CHAT_THREAD_REQ] = std::bind(&LogicSystem::getUserThreadsHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
	// 注册创建私聊回调函数
	fun_callbacks_[ID_CREATE_PRIVATE_CHAT_REQ] = std::bind(&LogicSystem::createPrivateChat, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
	// 注册加载聊天消息回调函数
	fun_callbacks_[ID_LOAD_CHAT_MSG_REQ] = std::bind(&LogicSystem::loadChatMsg, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
}

// 聊天登录回调函数
void LogicSystem::loginHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid is  " << uid << " user token  is "
		<< token << std::endl;

	Json::Value  rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->send(return_str, MSG_CHAT_LOGIN_RSP);
		});


	//从redis获取用户token是否正确
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisMgr::getInstance()->get(token_key, token_value);
	if (!success) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	rtvalue["error"] = ErrorCodes::Success;

	//此处添加分布式锁，让该线程独占登录
	//拼接用户ip对应的key
	auto lock_key = LOCK_PREFIX + uid_str;
	auto identifier = RedisMgr::getInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	//利用defer解锁
	Defer defer2([this, identifier, lock_key]() {
		RedisMgr::getInstance()->releaseLock(lock_key, identifier);
		});
	//此处判断该用户是否在别处或者本服务器登录

	std::string uid_ip_value = "";
	auto uid_ip_key = USERIPPREFIX + uid_str;
	bool b_ip = RedisMgr::getInstance()->get(uid_ip_key, uid_ip_value);
	//说明用户已经登录了，此处应该踢掉之前的用户登录状态
	if (b_ip) {
		//获取当前服务器ip信息
		auto& cfg = ConfigMgr::getInst();
		auto self_name = cfg["SelfServer"]["Name"];
		//如果之前登录的服务器和当前相同，则直接在本服务器踢掉
		if (uid_ip_value == self_name) {
			//查找旧有的连接
			auto old_session = UserMgr::getInstance()->getSession(uid);

			//此处应该发送踢人消息
			if (old_session) {
				std::cout << "kick user " << uid << " from old session " << old_session->getUuid() << std::endl;
				old_session->notifyOffline(uid);
				//清除旧的连接
				p_server_->clearSession(old_session->getUuid());
			}

		}
		else {
			//如果不是本服务器，则通知grpc通知其他服务器踢掉
			//发送通知
			KickUserReq kick_req;
			kick_req.set_uid(uid);
			std::cout << "send kick user " << uid << " msg to peer server" << std::endl;
			ChatGrpcClient::getInstance()->notifyKickUser(uid_ip_value, kick_req);
		}
	}


	std::string base_key = USER_BASE_INFO + uid_str;
	auto user_info = std::make_shared<UserInfo>();
	bool b_base = getBaseInfo(base_key, uid, user_info);
	if (!b_base) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;

	//从数据库获取申请列表
	std::vector<std::shared_ptr<ApplyInfo>> apply_list;
	auto b_apply = getFriendApplyInfo(uid, apply_list);
	if (b_apply) {
		for (auto& apply : apply_list) {
			Json::Value obj;
			obj["name"] = apply->_name;
			obj["uid"] = apply->_uid;
			obj["icon"] = apply->_icon;
			obj["nick"] = apply->_nick;
			obj["sex"] = apply->_sex;
			obj["desc"] = apply->_desc;
			obj["status"] = apply->_status;
			rtvalue["apply_list"].append(obj);
		}
	}

	//获取好友列表
	std::vector<std::shared_ptr<UserInfo>> friend_list;
	bool b_friend_list = getFriendList(uid, friend_list);
	for (auto& friend_ele : friend_list) {
		Json::Value obj;
		obj["name"] = friend_ele->name;
		obj["uid"] = friend_ele->uid;
		obj["icon"] = friend_ele->icon;
		obj["nick"] = friend_ele->nick;
		obj["sex"] = friend_ele->sex;
		obj["desc"] = friend_ele->desc;
		obj["back"] = friend_ele->back;
		rtvalue["friend_list"].append(obj);
	}

	auto server_name = ConfigMgr::getInst().getValue("SelfServer", "Name");
	//将登录数量增加
	auto rd_res = RedisMgr::getInstance()->hGet(LOGIN_COUNT, server_name);
	int count = 0;
	if (!rd_res.empty()) {
		count = std::stoi(rd_res);
	}

	count++;
	auto count_str = std::to_string(count);
	RedisMgr::getInstance()->hSet(LOGIN_COUNT, server_name, count_str);

	//session绑定用户uid
	session->setUserId(uid);
	//为用户设置登录ip server的名字
	std::string  ipkey = USERIPPREFIX + uid_str;
	RedisMgr::getInstance()->set(ipkey, server_name);
	//uid和session绑定管理,方便以后踢人操作
	UserMgr::getInstance()->setUserSession(uid, session);
	std::string  uid_session_key = USER_SESSION_PREFIX + uid_str;
	RedisMgr::getInstance()->set(uid_session_key, session->getUuid());
	return;
}

// 查找好友回调函数：根据用户uid查询具体信息
void LogicSystem::searchInfo(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid_str = root["uid"].asString();
	std::cout << "user SearchInfo uid is  " << uid_str << std::endl;

	Json::Value rtvalue;

	Defer deder([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->send(return_str, ID_SEARCH_USER_RSP);
		});

	bool b_digit = isPureDigit(uid_str);
	if (b_digit) {
		getUserByUid(uid_str, rtvalue);
	}
	else {
		getUserByName(uid_str, rtvalue);
	}
}

// 添加好友回调函数
void LogicSystem::addFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto desc = root["applyname"].asString();
	auto bakname = root["bakname"].asString();
	auto touid = root["touid"].asInt();
	std::cout << "user login uid is  " << uid << " applydesc  is "
		<< desc << " bakname is " << bakname << " touid is " << touid << endl;

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->send(return_str, ID_ADD_FRIEND_RSP);
		});

	//先更新数据库
	MysqlMgr::getInstance()->addFriendApply(uid, touid, desc, bakname);

	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::getInstance()->get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}


	auto& cfg = ConfigMgr::getInst();
	auto self_name = cfg["SelfServer"]["Name"];


	std::string base_key = USER_BASE_INFO + std::to_string(uid);
	auto apply_info = std::make_shared<UserInfo>();
	bool b_info = getBaseInfo(base_key, uid, apply_info);

	//直接通知对方有申请消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::getInstance()->getSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["applyuid"] = uid;
			notify["name"] = apply_info->name;
			notify["desc"] = desc;
			if (b_info) {
				notify["icon"] = apply_info->icon;
				notify["sex"] = apply_info->sex;
				notify["nick"] = apply_info->nick;
			}
			std::string return_str = notify.toStyledString();
			session->send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
		}

		return;
	}


	AddFriendReq add_req;
	add_req.set_applyuid(uid);
	add_req.set_touid(touid);
	add_req.set_name(apply_info->name);
	add_req.set_desc(desc);
	if (b_info) {
		add_req.set_icon(apply_info->icon);
		add_req.set_sex(apply_info->sex);
		add_req.set_nick(apply_info->nick);
	}

	//发送通知
	ChatGrpcClient::getInstance()->notifyAddFriend(to_ip_value, add_req);

}

// 同意好友申请信息回调函数
void LogicSystem::authFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {

	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);

	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();
	auto back_name = root["back"].asString();
	std::cout << "from " << uid << " auth friend to " << touid << std::endl;

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	auto user_info = std::make_shared<UserInfo>();

	std::string base_key = USER_BASE_INFO + std::to_string(touid);
	bool b_info = getBaseInfo(base_key, touid, user_info);
	if (b_info) {
		rtvalue["name"] = user_info->name;
		rtvalue["nick"] = user_info->nick;
		rtvalue["icon"] = user_info->icon;
		rtvalue["sex"] = user_info->sex;
		rtvalue["uid"] = touid;
	}
	else {
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}


	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->send(return_str, ID_AUTH_FRIEND_RSP);
		});

	//先更新数据库， 放到事务中，此处不再处理
	//MysqlMgr::GetInstance()->AuthFriendApply(uid, touid);

	std::vector<std::shared_ptr<AddFriendMsg>> chat_datas;

	//更新数据库添加好友
	MysqlMgr::getInstance()->addFriend(uid, touid, back_name, chat_datas);

	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::getInstance()->get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}

	auto& cfg = ConfigMgr::getInst();
	auto self_name = cfg["SelfServer"]["Name"];
	//直接通知对方有认证通过消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::getInstance()->getSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["fromuid"] = uid;
			notify["touid"] = touid;
			std::string base_key = USER_BASE_INFO + std::to_string(uid);
			auto user_info = std::make_shared<UserInfo>();
			bool b_info = getBaseInfo(base_key, uid, user_info);
			if (b_info) {
				notify["name"] = user_info->name;
				notify["nick"] = user_info->nick;
				notify["icon"] = user_info->icon;
				notify["sex"] = user_info->sex;
			}
			else {
				notify["error"] = ErrorCodes::UidInvalid;
			}

			for (auto& chat_data : chat_datas)
			{
				Json::Value  chat;
				chat["sender"] = chat_data->sender_id();
				chat["msg_id"] = chat_data->msg_id();
				chat["thread_id"] = chat_data->thread_id();
				chat["unique_id"] = chat_data->unique_id();
				chat["msg_content"] = chat_data->msgcontent();
				notify["chat_datas"].append(chat);
				rtvalue["chat_datas"].append(chat);
			}

			std::string return_str = notify.toStyledString();
			session->send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
		}

		return;
	}


	AuthFriendReq auth_req;
	auth_req.set_fromuid(uid);
	auth_req.set_touid(touid);
	for (auto& chat_data : chat_datas)
	{
		auto text_msg = auth_req.add_textmsgs();
		text_msg->CopyFrom(*chat_data);
		Json::Value  chat;
		chat["sender"] = chat_data->sender_id();
		chat["msg_id"] = chat_data->msg_id();
		chat["thread_id"] = chat_data->thread_id();
		chat["unique_id"] = chat_data->unique_id();
		chat["msg_content"] = chat_data->msgcontent();
		rtvalue["chat_datas"].append(chat);
	}
	//发送通知
	ChatGrpcClient::getInstance()->notifyAuthFriend(to_ip_value, auth_req);
}

// 发送信息回调函数
void LogicSystem::dealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);

	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();

	const Json::Value  arrays = root["text_array"];

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["fromuid"] = uid;
	rtvalue["touid"] = touid;
	auto thread_id = root["thread_id"].asInt();
	rtvalue["thread_id"] = thread_id;
	std::vector<std::shared_ptr<ChatMessage>> chat_datas;
	auto timestamp = getCurrentTimestamp();
	for (const auto& txt_obj : arrays) {
		auto content = txt_obj["content"].asString();
		auto unique_id = txt_obj["unique_id"].asString();
		std::cout << "content is " << content << std::endl;
		std::cout << "unique_id is " << unique_id << std::endl;
		auto chat_msg = std::make_shared<ChatMessage>();
		chat_msg->chat_time = timestamp;
		chat_msg->sender_id = uid;
		chat_msg->recv_id = touid;
		chat_msg->unique_id = unique_id;
		chat_msg->thread_id = thread_id;
		chat_msg->content = content;
		chat_msg->status = 2;
		chat_msg->msg_type = int(ChatMsgType::TEXT);
		chat_datas.push_back(chat_msg);
	}

	//插入数据库
	MysqlMgr::getInstance()->addChatMsg(chat_datas);

	for (const auto& chat_data : chat_datas) {
		Json::Value  chat_msg;
		chat_msg["message_id"] = chat_data->message_id;
		chat_msg["unique_id"] = chat_data->unique_id;
		chat_msg["content"] = chat_data->content;
		chat_msg["status"] = chat_data->status;
		chat_msg["chat_time"] = chat_data->chat_time;
		rtvalue["chat_datas"].append(chat_msg);
	}

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->send(return_str, ID_TEXT_CHAT_MSG_RSP);
		});


	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::getInstance()->get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}

	auto& cfg = ConfigMgr::getInst();
	auto self_name = cfg["SelfServer"]["Name"];
	// 如果对方在本服务器，直接通知对方有认证通过消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::getInstance()->getSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			std::string return_str = rtvalue.toStyledString();
			session->send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
		}

		return;
	}


	TextChatMsgReq text_msg_req;
	text_msg_req.set_fromuid(uid);
	text_msg_req.set_touid(touid);
	text_msg_req.set_thread_id(thread_id);
	for (const auto& chat_data : chat_datas) {
		auto* text_msg = text_msg_req.add_textmsgs();
		text_msg->set_unique_id(chat_data->unique_id);
		text_msg->set_msgcontent(chat_data->content);
		text_msg->set_msg_id(chat_data->message_id);
		text_msg->set_chat_time(chat_data->chat_time);
	}


	//发送通知 todo...
	ChatGrpcClient::getInstance()->notifyTextChatMsg(to_ip_value, text_msg_req, rtvalue);
}

// 心跳处理回调函数
void LogicSystem::heartBeatHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["fromuid"].asInt();
	std::cout << "receive heart beat msg, uid is " << uid << std::endl;
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	session->send(rtvalue.toStyledString(), ID_HEARTBEAT_RSP);
}

// 加载聊天线程记录
void LogicSystem::getUserThreadsHandler(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	//从数据库加chat_threads记录
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	int last_id = root["thread_id"].asInt();
	std::cout << "get uid  threads  " << uid << std::endl;

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["uid"] = uid;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->send(return_str, ID_LOAD_CHAT_THREAD_RSP);
		});

	std::vector<std::shared_ptr<ChatThreadInfo>> threads;

	int page_size = 10;
	bool load_more = false;
	int64_t next_last_id = 0;
	bool res = getUserThreads(uid, last_id, page_size, threads, load_more, next_last_id);
	if (!res) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	rtvalue["load_more"] = load_more;
	rtvalue["next_last_id"] = (int)next_last_id;
	//整理threads数据写入json返回
	for (auto& thread : threads) {
		Json::Value thread_value;
		thread_value["thread_id"] = int(thread->_thread_id);
		thread_value["type"] = thread->_type;
		thread_value["user1_id"] = thread->_user1_id;
		thread_value["user2_id"] = thread->_user2_id;
		rtvalue["threads"].append(thread_value);
	}
}

// 创建私聊回调函数
void LogicSystem::createPrivateChat(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto other_id = root["other_id"].asInt();

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["uid"] = uid;
	rtvalue["other_id"] = other_id;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->send(return_str, ID_LOAD_CHAT_THREAD_RSP);
		});

	int thread_id = 0;
	bool res = MysqlMgr::getInstance()->createPrivateChat(uid, other_id, thread_id);
	if (!res) {
		rtvalue["error"] = ErrorCodes::CreatChatFailed;
		return;
	}

	rtvalue["thread_id"] = thread_id;
}


void LogicSystem::loadChatMsg(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto thread_id = root["thread_id"].asInt();
	auto message_id = root["message_id"].asInt();

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["thread_id"] = thread_id;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->send(return_str, ID_LOAD_CHAT_MSG_RSP);
		});

	int page_size = 10;
	std::shared_ptr<PageResult> res = MysqlMgr::getInstance()->loadChatMsg(thread_id, message_id, page_size);
	if (!res) {
		rtvalue["error"] = ErrorCodes::LoadChatFailed;
		return;
	}

	rtvalue["last_message_id"] = res->next_cursor;
	rtvalue["load_more"] = res->load_more;
	for (auto& chat : res->messages) {
		Json::Value  chat_data;
		chat_data["sender"] = chat.sender_id;
		chat_data["message_id"] = chat.message_id;
		chat_data["thread_id"] = chat.thread_id;
		chat_data["unique_id"] = 0;
		chat_data["msg_content"] = chat.content;
		chat_data["chat_time"] = chat.chat_time;
		chat_data["status"] = chat.status;
		chat_data["msg_type"] = chat.msg_type;
		chat_data["receiver"] = chat.recv_id;
		rtvalue["chat_datas"].append(chat_data);
	}

}



// 获取用户的信息
bool LogicSystem::getBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {
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

// 检验字符串是否由纯数字组成
bool LogicSystem::isPureDigit(const std::string& str) {
	for (char c : str) {
		if (!std::isdigit(c)) {
			return false;
		}
	}
	return true;
}

// 根据uid查找用户
void LogicSystem::getUserByUid(std::string uid_str, Json::Value& rtvalue) {
	rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = USER_BASE_INFO + uid_str;

	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::getInstance()->get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << " icon is " << icon << std::endl;

		rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
	}

	auto uid = std::stoi(uid_str);
	//redis中没有则查询mysql
	//查询数据库
	std::shared_ptr<UserInfo> user_info = nullptr;
	user_info = MysqlMgr::getInstance()->getUser(uid);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	//将数据库内容写入redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMgr::getInstance()->set(base_key, redis_root.toStyledString());

	//返回数据
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
}

// 根据名字查找用户
void LogicSystem::getUserByName(std::string name, Json::Value& rtvalue) {
	rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = NAME_INFO + name;

	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::getInstance()->get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << std::endl;

		rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		return;
	}

	//redis中没有则查询mysql
	//查询数据库
	std::shared_ptr<UserInfo> user_info = nullptr;
	user_info = MysqlMgr::getInstance()->getUser(name);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	//将数据库内容写入redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;

	RedisMgr::getInstance()->set(base_key, redis_root.toStyledString());

	//返回数据
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
}

bool LogicSystem::getFriendApplyInfo(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list) {
	//从mysql获取好友申请列表
	return MysqlMgr::getInstance()->getApplyList(to_uid, list, 0, 10);
}

bool LogicSystem::getFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list) {
	//从mysql获取好友列表
	return MysqlMgr::getInstance()->getFriendList(self_id, user_list);
}


// 获取用户聊天线程
bool LogicSystem::getUserThreads(int64_t userId,
	int64_t lastId,
	int      pageSize,
	std::vector<std::shared_ptr<ChatThreadInfo>>& threads,
	bool& loadMore,
	int64_t& nextLastId) {
	return MysqlMgr::getInstance()->getUserThreads(userId, lastId, pageSize,
		threads, loadMore, nextLastId);
}