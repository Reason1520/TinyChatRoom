#include "usermgr.h"
#include "csession.h"

/******************************************************************************
 * @file       usermgr.cpp
 * @brief      用户管理类实现
 *
 * @author     lueying
 * @date       2026/1/29
 * @history
 *****************************************************************************/

UserMgr:: ~UserMgr() {
	uid_to_session_.clear();
}


std::shared_ptr<CSession> UserMgr::getSession(int uid) {
	std::lock_guard<std::mutex> lock(session_mtx_);
	auto iter = uid_to_session_.find(uid);
	if (iter == uid_to_session_.end()) {
		return nullptr;
	}

	return iter->second;
}

void UserMgr::setUserSession(int uid, std::shared_ptr<CSession> session) {
	std::lock_guard<std::mutex> lock(session_mtx_);
	uid_to_session_[uid] = session;
}

void UserMgr::rmvUserSession(int uid) {
	std::lock_guard<std::mutex> lock(session_mtx_);
	uid_to_session_.erase(uid);
}

UserMgr::UserMgr() {

}