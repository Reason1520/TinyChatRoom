#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

#include <map>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "singleton.h"
#include "const.h"
#include "data.h"
#include "csession.h"

/******************************************************************************
 * @file       logicsystem.h
 * @brief      逻辑系统类，实现消息的投递、业务处理等功能
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

class CServer;

using FunCallBack = std::function<void(std::shared_ptr<CSession>,
	const short& msg_id,
	const std::string& msg_data)>;

class LogicSystem :public Singleton<LogicSystem> {
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	// 将消息放入处理队列中
	void postMsgToQue(std::shared_ptr <LogicNode> msg);
	void setServer(std::shared_ptr<CServer> pserver);
private:
	LogicSystem();
	// 线程工作函数
	void dealMsg();
	// 注册处理回调函数
	void registerCallBacks();
	// 聊天登录回调函数
	void loginHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	// 查找好友回调函数
	void searchInfo(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	// 添加好友回调函数
	void addFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	// 同意好友申请信息回调函数
	void authFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	// 发送信息回调函数
	void dealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);

	std::thread worker_thread_;
	// 消息处理队列
	std::queue<std::shared_ptr<LogicNode>> msg_queue_;
	std::mutex mutex_;
	std::condition_variable consume_;

	bool b_stop_;
	std::map<short, FunCallBack> fun_callbacks_;
	std::shared_ptr<CServer> p_server_;

	// 存在内存中的用户数据
	std::unordered_map<int, std::shared_ptr<UserInfo>> users_;

	// 获取用户的信息
	bool getBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	// 检验字符串是否由纯数字组成
	bool isPureDigit(const std::string& str);
	// 根据uid查找用户
	void getUserByUid(std::string uid_str, Json::Value& rtvalue);
	// 根据名字查找用户
	void getUserByName(std::string name, Json::Value& rtvalue);
	// 获取好友申请信息
	bool getFriendApplyInfo(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list);
	// 获取好友列表信息
	bool getFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list);
};

#endif // LOGICSYSTEM_H

