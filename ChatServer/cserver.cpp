#include "cserver.h"
#include "asioioservicepool.h"
#include "usermgr.h"
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
acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
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