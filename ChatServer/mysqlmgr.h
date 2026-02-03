#ifndef MYSQLMGR_H
#define MYSQLMGR_H

#include "const.h"
#include "singleton.h"
#include "mysqldao.h"
#include "data.h"

/******************************************************************************
 * @file       mysqlmgr.h
 * @brief      mysql的服务层
 *
 * @author     lueying
 * @date       2026/1/1
 * @history
 *****************************************************************************/

class MysqlMgr : public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;
public:
    ~MysqlMgr();
    // 注册用户
    int regUser(const std::string& name, const std::string& email, const std::string& pwd);
    // 检查邮箱是否存在
    bool checkEmail(const std::string& name, const std::string& email);
    // 更新密码
    bool updatePwd(const std::string& name, const std::string& pwd);
    // 检查密码
    bool checkPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
    // 获取用户信息
    std::shared_ptr<UserInfo> getUser(int uid);
    std::shared_ptr<UserInfo> getUser(std::string name);
    // 添加添加好友请求
    bool addFriendApply(const int& from, const int& to);
    // 同意好友请求
    bool authFriendApply(const int& from, const int& to);
    // 添加联系人好友
    bool addFriend(const int& from, const int& to, std::string back_name);
    // 获取用户好友请求列表
    bool getApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit);
    // 获取用户好友列表
    bool getFriendList(int self_id, std::vector<std::shared_ptr<UserInfo> >& user_info);
private:
    MysqlMgr();
    MysqlDAO dao_;
};

#endif // MYSQLMGR_H