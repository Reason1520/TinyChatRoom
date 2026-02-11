#include "mysqlmgr.h"

/******************************************************************************
 * @file       mysqlmgr.h
 * @brief      mysql的服务层实现
 *
 * @author     lueying
 * @date       2026/1/1
 * @history
 *****************************************************************************/
MysqlMgr::MysqlMgr() {
}

MysqlMgr::~MysqlMgr() {

}

// 用户注册
int MysqlMgr::regUser(const std::string& name, const std::string& email, const std::string& pwd) {
    return dao_.regUser(name, email, pwd);
}

// 检查邮箱是否存在
bool MysqlMgr::checkEmail(const std::string& name, const std::string& email) {
    return dao_.checkEmail(name, email);
}

// 更新密码
bool MysqlMgr::updatePwd(const std::string& name, const std::string& pwd) {
    return dao_.updatePwd(name, pwd);
}

// 检查密码
bool MysqlMgr::checkPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo) {
    return dao_.checkPwd(name, pwd, userInfo);
}

// 获取用户信息
std::shared_ptr<UserInfo> MysqlMgr::getUser(int uid) {
    return dao_.getUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::getUser(std::string name) {
    return dao_.getUser(name);
}

// 添加添加好友请求
bool MysqlMgr::addFriendApply(const int& from, const int& to, const std::string& desc, const std::string& back_name) {
    return dao_.addFriendApply(from, to, desc, back_name);
}

// 同意好友请求
bool MysqlMgr::authFriendApply(const int& from, const int& to) {
    return dao_.authFriendApply(from, to);
}

// 添加联系人好友
bool MysqlMgr::addFriend(const int& from, const int& to, std::string back_name, std::vector<std::shared_ptr<AddFriendMsg>>& msg_list) {
    return dao_.addFriend(from, to, back_name, msg_list);
}

// 获取用户好友请求列表
bool MysqlMgr::getApplyList(int touid,
    std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit) {
    return dao_.getApplyList(touid, applyList, begin, limit);
}

// 获取用户好友列表
bool MysqlMgr::getFriendList(int self_id, std::vector<std::shared_ptr<UserInfo> >& user_info) {
    return dao_.getFriendList(self_id, user_info);
}

// 获取用户聊天线程
bool MysqlMgr::getUserThreads(int64_t userId,
    int64_t lastId,
    int      pageSize,
    std::vector<std::shared_ptr<ChatThreadInfo>>& threads,
    bool& loadMore,
    int64_t& nextLastId) {
    return dao_.getUserThreads(userId, lastId, pageSize, threads, loadMore, nextLastId);
}

// 创建私聊
bool MysqlMgr::createPrivateChat(int user1_id, int user2_id, int& thread_id) {
    return dao_.createPrivateChat(user1_id, user2_id, thread_id);
}

// 加载聊天消息
std::shared_ptr<PageResult> MysqlMgr::loadChatMsg(int threadId, int lastId, int pageSize) {
    return dao_.loadChatMsg(threadId, lastId, pageSize);
}

// 添加聊天消息
bool MysqlMgr::addChatMsg(std::vector<std::shared_ptr<ChatMessage>>& chat_datas) {
    return dao_.addChatMsg(chat_datas);
}

bool MysqlMgr::addChatMsg(std::shared_ptr<ChatMessage> chat_data) {
    return dao_.addChatMsg(chat_data);
}

// 获取聊天信息
std::shared_ptr<ChatMessage> MysqlMgr::getChatMsg(int message_id) {
    return dao_.getChatMsg(message_id);
}