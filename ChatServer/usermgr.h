#ifndef USERMGR_H_
#define USERMGR_H_

#include "singleton.h"
#include <unordered_map>
#include <memory>
#include <mutex>

/******************************************************************************
 * @file       usermgr.h
 * @brief      用户管理类，将连接(session)和用户uid绑定
 *
 * @author     lueying
 * @date       2026/1/29
 * @history
 *****************************************************************************/

class CSession;
class UserMgr : public Singleton<UserMgr>
{
	friend class Singleton<UserMgr>;
public:
	~UserMgr();
	// 查询
	std::shared_ptr<CSession> getSession(int uid);
	// 上线：将uid和session存入哈希表
	void setUserSession(int uid, std::shared_ptr<CSession> session);
	// 离线：从表中删除对应的 uid
	void rmvUserSession(int uid);
private:
	UserMgr();
	std::mutex session_mtx_;
	std::unordered_map<int, std::shared_ptr<CSession>> uid_to_session_;	// uid到session的映射表
};


#endif // USERMGR_H_