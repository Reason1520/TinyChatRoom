#ifndef CSESSION_H
#define CSESSION_H

#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <iostream>
#include <queue>
#include "const.h"
#include "msgnode.h"

/******************************************************************************
 * @file       csession.h
 * @brief      会话类
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

using boost::asio::ip::tcp;

class CServer;
class LogicSystem;

class CSession : public std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context& io_context, CServer* server);
    ~CSession();

    tcp::socket& getSocket();
    std::string& getUuid();
    void setUserId(int uid);
    int getUserId();
    std::shared_ptr<CSession> sharedSelf();
    void start();
    void send(char* msg, short max_length, short msgid);
    void send(std::string msg, short msgid);
    void close();

private:
    tcp::socket socket_;
    std::string uuid_;
    CServer* server_;
    int user_uid_;

    bool b_head_pares_; // 是否解析头部
    bool b_close_;      // 是否关闭连接

    std::mutex send_lock_;                          // 发送队列锁
    std::queue<std::shared_ptr<MsgNode>> send_queue_;    // 发送队列

    // 协议解析相关的节点
    std::shared_ptr<MsgNode> recv_head_node_;
    std::shared_ptr<RecvNode> recv_msg_node_;

    // 异步读逻辑
    void asyncReadHead(int total_len);
    void asyncReadBody(int total_len);

    // 辅助读取函数
    // 读取完整长度
    void asyncReadFull(std::size_t maxLength,
        std::function<void(const boost::system::error_code&, std::size_t)> handler);
    // 读取指定范围
    void asyncReadLen(std::size_t read_len, std::size_t total_len,
        std::function<void(const boost::system::error_code&, std::size_t)> handler);

    // 读写处理函数
    void handleRead(const boost::system::error_code& error, size_t  bytes_transferred, std::shared_ptr<CSession> shared_self);
    void handleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared);

    // 接收缓冲区
    char data_[MAX_LENGTH]; // MAX_LENGTH 建议定义在 const.h
};


// 提交给logicsystem的事务
class LogicNode {
    friend class LogicSystem;
public:
    LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);
private:
    std::shared_ptr<CSession> session_;
    std::shared_ptr<RecvNode> recvnode_;
};

#endif // CSESSION_H
