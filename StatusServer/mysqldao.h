#ifndef MYSQLDAO_H
#define MYSQLDAO_H
#include "const.h"
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
private:
    std::unique_ptr<MysqlPool> pool_;
};

#endif // MYSQLDAO_H