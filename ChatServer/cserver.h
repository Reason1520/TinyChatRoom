#ifndef CSERVER_H
#define CSERVER_H

#include <boost/asio.hpp>
#include "csession.h"
#include <memory.h>
#include <map>
#include <mutex>
#include <boost/asio/steady_timer.hpp>

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

class CServer: public std::enable_shared_from_this<CServer> {
public:
    CServer(boost::asio::io_context& io_context, short port);
    ~CServer();
    void clearSession(std::string);
    // 检查server中是否有指定uuid的session
    bool checkValid(std::string uuid);
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
    boost::asio::steady_timer timer_;

    // 定时心跳监测
    void on_timer(const boost::system::error_code& e);
};

#endif // CSERVER_H