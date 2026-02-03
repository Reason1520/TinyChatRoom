#ifndef CSERVER_H
#define CSERVER_H

#include <boost/asio.hpp>
#include "csession.h"
#include <memory.h>
#include <map>
#include <mutex>

/******************************************************************************
 * @file       cserver.h
 * @brief      chat server服务器核心类
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

using namespace std;
using boost::asio::ip::tcp;

class CServer {
public:
    CServer(boost::asio::io_context& io_context, short port);
    ~CServer();
    void clearSession(std::string);
private:
    // 监听与处理新连接请求
    void startAccept();
    // 连接回调处理
    void handleAccept(shared_ptr<CSession>, const boost::system::error_code& error);
    boost::asio::io_context& io_context_;
    short port_;
    tcp::acceptor acceptor_;
    // 会话管理
    std::map<std::string, shared_ptr<CSession>> sessions_;
    std::mutex mutex_;  // 保护会话
};

#endif // CSERVER_H