#ifndef MYSQLDAO_H
#define MYSQLDAO_H

#include "const.h"
#include "data.h"
#include <string>
#include <memory>
#include <iostream>

/******************************************************************************
 * @file       mysqldao.h
 * @brief      mysql的Data Access Object数据库连接管理
 *
 * @author     lueying
 * @date       2025/12/30
 * @history
 *****************************************************************************/
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
    bool addFriendApply(const int& from, const int& to);
    // 同意好友请求
    bool authFriendApply(const int& from, const int& to);
    // 添加联系人好友
    bool addFriend(const int& from, const int& to, std::string back_name);
    // 获取好友请求列表
    bool getApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit);
    // 获取用户好友列表
    bool getFriendList(int self_id, std::vector<std::shared_ptr<UserInfo> >& user_info_list);

private:
    std::unique_ptr<MysqlPool> pool_;
};

#endif // MYSQLDAO_H