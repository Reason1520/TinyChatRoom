#include "cserver.h"
#include "asioioservicepool.h"
#include "usermgr.h"
#include "redismgr.h"
#include "configmgr.h"
#include <iostream>

/******************************************************************************
 * @file       cserver.h
 * @brief      chat server服务器核心类实现
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

CServer::CServer(boost::asio::io_context& io_context, short port) :io_context_(io_context), port_(port),
acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), timer_(io_context) {
    std::cout << "Server start success, listen on port : " << port_ << endl;
    startAccept();
}

CServer::~CServer() {
}

// 监听与处理新连接请求
void CServer::startAccept() {
    auto& io_context = AsioIOServicePool::getInstance()->getIOService();
    shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);
    acceptor_.async_accept(new_session->getSocket(), std::bind(&CServer::handleAccept, this, new_session, placeholders::_1));   // 也可以用lamda表达式
}

// 接受连接回调处理
void CServer::handleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->start();
        lock_guard<mutex> lock(mutex_);
        sessions_.insert(make_pair(new_session->getUuid(), new_session));
    }
    else {
        cout << "session accept failed, error is " << error.what() << endl;
    }

    startAccept();
}

// 清理连接
void CServer::clearSession(std::string session_id) {
    if (sessions_.find(session_id) != sessions_.end()) {
        //移除用户和session的关联
        UserMgr::getInstance()->rmvUserSession(sessions_[session_id]->getUserId());
    }

    {
        lock_guard<mutex> lock(mutex_);
        sessions_.erase(session_id);
    }
}

// 定时心跳监测
void CServer::on_timer(const boost::system::error_code& ec) {
    std::vector<std::shared_ptr<CSession>> _expired_sessions;
    int session_count = 0;
    //此处加锁遍历session
    {
        lock_guard<mutex> lock(mutex_);
        time_t now = std::time(nullptr);
        for (auto iter = sessions_.begin(); iter != sessions_.end(); iter++) {
            auto b_expired = iter->second->isHeartbeatExpired(now);
            if (b_expired) {
                //关闭socket, 其实这里也会触发async_read的错误处理
                iter->second->close();
                //收集过期信息
                _expired_sessions.push_back(iter->second);
                continue;
            }
            session_count++;
        }
    }

    //设置session数量
    auto& cfg = ConfigMgr::getInst();
    auto self_name = cfg["SelfServer"]["Name"];
    auto count_str = std::to_string(session_count);
    RedisMgr::getInstance()->hSet(LOGIN_COUNT, self_name, count_str);

    //处理过期session, 单独提出，防止死锁
    for (auto& session : _expired_sessions) {
        session->dealExceptionSession();
    }

    //再次设置，下一个60s检测
    timer_.expires_after(std::chrono::seconds(60));
    timer_.async_wait([this](boost::system::error_code ec) {
        on_timer(ec);
        });
}

// 检查server中是否有指定uuid的session
bool CServer::checkValid(std::string uuid) {
    lock_guard<mutex> lock(mutex_);
    auto it = sessions_.find(uuid);
    if (it != sessions_.end()) {
        return true;
    }
    return false;
}