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