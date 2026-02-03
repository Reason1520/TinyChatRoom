#ifndef MYSQLMGR_H
#define MYSQLMGR_H

#include "const.h"
#include "singleton.h"
#include "mysqldao.h"

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
private:
    MysqlMgr();
    MysqlDAO dao_;
};

#endif // MYSQLMGR_H