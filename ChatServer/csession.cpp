#include "csession.h"
#include "cserver.h"
#include "logicsystem.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

/******************************************************************************
 * @file       csession.h
 * @brief      会话类实现
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

CSession::CSession(boost::asio::io_context& io_context, CServer* server): 
    socket_(io_context), server_(server), b_close_(false), b_head_pares_(false), user_uid_(0) {
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    uuid_ = boost::uuids::to_string(a_uuid);
    recv_head_node_ = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

CSession::~CSession() {
}

tcp::socket& CSession::getSocket() {
    return socket_;
}

std::string& CSession::getUuid() {
    return uuid_;
}

void CSession::setUserId(int uid) {
    user_uid_ = uid;
}

int CSession::getUserId() {
    return user_uid_;
}

std::shared_ptr<CSession> CSession::sharedSelf() {
    return shared_from_this();
}


// 开始
void CSession::start() {
    asyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::close() {
    socket_.close();
    b_close_ = true;
}

// 异步读取消息头部
void CSession::asyncReadHead(int total_len) {
    auto self = shared_from_this();
    asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
        try {
            if (ec) {
                // 如果出现错误，关闭连接
                std::cout << "handle read failed, error is " << ec.what() << std::endl;
                close();
                server_->clearSession(uuid_);
                return;
            }

            if (bytes_transfered < HEAD_TOTAL_LEN) {
                // 如果读取头部长度不够
                std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
                    << HEAD_TOTAL_LEN << "]" << std::endl;
                close();
                server_->clearSession(uuid_);
                return;
            }

            // 读取头部成功，将缓冲区中的头部数据拷贝到RecvHeadNode中
            recv_head_node_->clear();
            memcpy(recv_head_node_->data_, data_, bytes_transfered);

            //获取头部MSGID数据
            short msg_id = 0;
            memcpy(&msg_id, recv_head_node_->data_, HEAD_ID_LEN);

            //网络字节序转化为本地字节序
            msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
            std::cout << "msg_id is " << msg_id << std::endl;

            // id非法
            if (msg_id > MAX_LENGTH) {
                std::cout << "invalid msg_id is " << msg_id << std::endl;
                server_->clearSession(uuid_);
                return;
            }

            // 读取数据长度数据
            short msg_len = 0;
            memcpy(&msg_len, recv_head_node_->data_ + HEAD_ID_LEN, HEAD_DATA_LEN);

            //网络字节序转化为本地字节序
            msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
            std::cout << "msg_len is " << msg_len << std::endl;

            // 数据长度非法（超过缓冲区长度）
            if (msg_len > MAX_LENGTH) {
                std::cout << "invalid data length is " << msg_len << std::endl;
                server_->clearSession(uuid_);
                return;
            }

            recv_msg_node_ = std::make_shared<RecvNode>(msg_len, msg_id);
            // 读取数据体
            asyncReadBody(msg_len);
        }
        catch (std::exception& e) {
            std::cout << "Exception code is " << e.what() << std::endl;
        }
        });
}

//读取完整长度
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    // 清空缓冲区
    ::memset(data_, 0, MAX_LENGTH);
    asyncReadLen(0, maxLength, handler);
}

//读取指定字节范围（解决半包问题）
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len,
    std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    auto self = shared_from_this();
    socket_.async_read_some(boost::asio::buffer(data_ + read_len, total_len - read_len),
        [read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
            if (ec) {
                // 出现错误，调用回调函数
                handler(ec, read_len + bytesTransfered);
                return;
            }

            if (read_len + bytesTransfered >= total_len) {
                //长度够了就调用回调函数
                handler(ec, read_len + bytesTransfered);
                return;
            }

            // 长度不够，继续递归调用自己，直到读满 total_len
            self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
        });
}

// 读取数据体
void CSession::asyncReadBody(int total_len) {
    auto self = shared_from_this();
    asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
        try {
            if (ec) {
                std::cout << "handle read failed, error is " << ec.what() << std::endl;
                close();
                server_->clearSession(uuid_);
                return;
            }

            if (bytes_transfered < total_len) {
                std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
                    << total_len << "]" << std::endl;
                close();
                server_->clearSession(uuid_);
                return;
            }

            // 将缓冲区中的数据拷贝到RecvNode中
            memcpy(recv_msg_node_->data_, data_, bytes_transfered);
            recv_msg_node_->cur_len_ += bytes_transfered;
            recv_msg_node_->data_[recv_msg_node_->total_len_] = '\0';
            std::cout << "receive data is " << recv_msg_node_->data_ << std::endl;
            //此处将消息投递到逻辑队列中
            LogicSystem::getInstance()->postMsgToQue(make_shared<LogicNode>(shared_from_this(), recv_msg_node_));
            //继续监听头部接受事件
            asyncReadHead(HEAD_TOTAL_LEN);
        }
        catch (std::exception& e) {
            std::cout << "Exception code is " << e.what() << std::endl;
        }
    });
}

// 发送数据
void CSession::send(char* msg, short max_length, short msgid) {
    std::lock_guard<std::mutex> lock(send_lock_);
    int send_que_size = send_queue_.size();
    if (send_que_size > MAX_SENDQUE) {
        std::cout << "session: " << uuid_ << " send que fulled, size is " << MAX_SENDQUE << endl;
        return;
    }

    send_queue_.push(make_shared<SendNode>(msg, max_length, msgid));
    if (send_que_size > 0) {
        // 如果压入前，发送队列中已经有消息，说明正在发送，直接返回即可
        return;
    }
    // 如果压入前发送队列为空，说明当前没有消息在发送，开始发送
    auto& msgnode = send_queue_.front();
    boost::asio::async_write(socket_, boost::asio::buffer(msgnode->data_, msgnode->total_len_),
        std::bind(&CSession::handleWrite, this, std::placeholders::_1, sharedSelf()));
}

void CSession::send(std::string msg, short msgid) {
    std::lock_guard<std::mutex> lock(send_lock_);
    int send_que_size = send_queue_.size();
    if (send_que_size > MAX_SENDQUE) {
        std::cout << "session: " << uuid_ << " send que fulled, size is " << MAX_SENDQUE << endl;
        return;
    }

    send_queue_.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
    if (send_que_size > 0) {
        return;
    }
    auto& msgnode = send_queue_.front();
    boost::asio::async_write(socket_, boost::asio::buffer(msgnode->data_, msgnode->total_len_),
        std::bind(&CSession::handleWrite, this, std::placeholders::_1, sharedSelf()));
}

// 写处理函数
void CSession::handleWrite(const boost::system::error_code& error, shared_ptr<CSession> self_shared) {
    try {
        if (!error) {
            std::lock_guard<std::mutex> lock(send_lock_);
            send_queue_.pop();
            if (!send_queue_.empty()) {
                // 如果当前发送队列仍然有数据，继续发送
                auto& msgnode = send_queue_.front();
                boost::asio::async_write(socket_, boost::asio::buffer(msgnode->data_, msgnode->total_len_),
                    std::bind(&CSession::handleWrite, this, std::placeholders::_1, self_shared));
            }
        }
        else {
            std::cout << "handle write failed, error is " << error.what() << endl;
            close();
            server_->clearSession(uuid_);
        }
    }
    catch (std::exception& e) {
        std::cout << "Exception code :" << e.what() << std::endl;
    }
}


LogicNode::LogicNode(std::shared_ptr<CSession>  session,
    std::shared_ptr<RecvNode> recvnode) :session_(session), recvnode_(recvnode) {
}