#ifndef MYSQLDAO_H
#define MYSQLDAO_H

#include "const.h"
#include "data.h"
#include <string>
#include <memory>
#include <iostream>
#include "message.grpc.pb.h"
#include "message.pb.h"

/******************************************************************************
 * @file       mysqldao.h
 * @brief      mysql的Data Access Object数据库连接管理
 *
 * @author     lueying
 * @date       2025/12/30
 * @history
 *****************************************************************************/

using message::AddFriendMsg;

class MysqlPool;

class MysqlDAO {
public:
    MysqlDAO();
    ~MysqlDAO();
    // 用户注册登记
    int regUser(const std::string& name, const std::string& email, const std::string& pwd);
    // 检查邮箱是否存在
    bool checkEmail(const std::string& name, const std::string& email);
    // 更新密码
    bool updatePwd(const std::string& name, const std::string& newpwd);
    // 检查密码
    bool checkPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
    // 获取用户信息
    std::shared_ptr<UserInfo> getUser(int uid);
    std::shared_ptr<UserInfo> getUser(std::string name);
    // 添加添加好友请求
    bool addFriendApply(const int& from, const int& to, const std::string& desc, const std::string& back_name);
    // 同意好友请求
    bool authFriendApply(const int& from, const int& to);
    // 添加联系人好友
    bool addFriend(const int& from, const int& to, std::string back_name, std::vector<std::shared_ptr<AddFriendMsg>>& chat_datas);
    // 获取好友请求列表
    bool getApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit);
    // 获取用户好友列表
    bool getFriendList(int self_id, std::vector<std::shared_ptr<UserInfo> >& user_info_list);
    // 获取用户从lastid开始的聊天线程
    bool getUserThreads(int64_t userId, int64_t lastId, int pageSize, std::vector<std::shared_ptr<ChatThreadInfo>>& threads,
        bool& loadMore, int64_t& nextLastId);
    // 创建私聊
    bool createPrivateChat(int user1_id, int user2_id, int& thread_id);
    // 加载聊天消息
    std::shared_ptr<PageResult> loadChatMsg(int threadId, int lastId, int pageSize);
    // 添加聊天消息
    bool addChatMsg(std::vector<std::shared_ptr<ChatMessage>>& chat_datas);
    bool addChatMsg(std::shared_ptr<ChatMessage> chat_data);
    // 获取聊天信息
    std::shared_ptr<ChatMessage> getChatMsg(int message_id);
private:
    std::unique_ptr<MysqlPool> pool_;
};

#endif // MYSQLDAO_H