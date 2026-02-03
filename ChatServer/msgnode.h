#ifndef MSG_NODE_H
#define MSG_NODE_H

#include <cstring>
#include <iostream>

/******************************************************************************
 * @file       msgnode.h
 * @brief      消息节点类
 *
 * @author     lueying
 * @date       2026/1/5
 * @history
 *****************************************************************************/

class MsgNode {
public:
    MsgNode(short max_len) :total_len_(max_len), cur_len_(0) {
        data_ = new char[total_len_ + 1]();
        data_[total_len_] = '\0';
    }

    ~MsgNode() {
        std::cout << "destruct MsgNode" << std::endl;
        delete[] data_;
    }

    void clear() {
        ::memset(data_, 0, total_len_);
        cur_len_ = 0;
    }

    short cur_len_;
    short total_len_;
    char* data_;
};

class RecvNode :public MsgNode {
    friend class LogicSystem;
public:
    RecvNode(short max_len, short msg_id);
private:
    short msg_id_;
};

class SendNode :public MsgNode {
    friend class LogicSystem;
public:
    SendNode(const char* msg, short max_len, short msg_id);
private:
    short msg_id_;
};

#endif // MSG_NODE_H_