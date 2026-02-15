#ifndef TCPMGR_H
#define TCPMGR_H

#include <QTcpSocket>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QQueue>
#include "singleton.h"
#include "global.h"
#include "userdata.h"

/******************************************************************************
 * @file       tcp.h
 * @brief      服务器和客户端长连接TCP管理类，使用独立线程
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

class TcpThread :public std::enable_shared_from_this<TcpThread> {
public:
    TcpThread();
    ~TcpThread();
private:
    QThread* tcp_thread_;
};

class TcpMgr :public QObject, public Singleton<TcpMgr>,
    public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    TcpMgr();
    // 关闭链接
    void closeConnection();
private:
    QTcpSocket socket_;
    QString host_;
    uint16_t port_;
    QByteArray buffer_;
    bool b_recv_pending_;   // 是否有未接收完的数据包
    quint16 message_id_;
    quint16 message_len_;
    // 消息处理回调函数
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> handlers_;
    //发送队列
    QQueue<QByteArray> send_queue_;
    //正在发送的包
    QByteArray current_block_;
    //当前已发送的字节数
    qint64 bytes_sent_;
    //是否正在发送
    bool pending_;

    void initHandlers();
    // 处理消息
    void handleMsg(ReqId id, int len, QByteArray data);
    // 注册自定义的类型
    void registerMetaType();

public slots:
    // 连接对端服务器
    void slot_tcp_connect(std::shared_ptr<ServerInfo> si);
    void slot_send_data(ReqId reqId, QByteArray data);
    void slot_tcp_close();

signals:
    void sig_con_success(bool bsuccess);
    void sig_send_data(ReqId reqId, QByteArray data);
    void sig_login_failed(int);
    void sig_swich_chatdlg();
    void sig_user_search(std::shared_ptr<SearchInfo>);      // 用户搜索完成信号
    void sig_load_apply_list(QJsonArray json_array);        // 加载好友申请列表完成信号
    void sig_friend_apply(std::shared_ptr<AddFriendApply>); // 好友申请完成信号
    void sig_add_auth_friend(std::shared_ptr<AuthInfo>);    // 同意好友申请完成信号（同意方）
    void sig_auth_rsp(std::shared_ptr<AuthRsp>);            // 收到同意好友申请信号（发送申请方）
    void sig_text_chat_msg(std::vector<std::shared_ptr<TextChatData>> msg_list);    // 接收到聊天消息信号
    void sig_notify_offline();                              // 通知客户端下线
    void sig_close();                                       // 关闭连接信号
    void sig_connection_closed();                           // 连接断开信号
    void sig_load_chat_thread(bool load_more, int last_thread_id,
        std::vector<std::shared_ptr<ChatThreadInfo>> chat_list);    // 加载聊天线程完成信号
    void sig_create_private_chat(int uid, int other_id, int thread_id); // 创建私聊完成信号
    void sig_load_chat_msg(int thread_id, int last_msg_id, bool load_more, 
        std::vector<std::shared_ptr<ChatDataBase>> chat_datas); // 加载聊天消息完成信号
    void sig_chat_msg_rsp(int thread_id,
        std::vector<std::shared_ptr<TextChatData>> chat_datas); // 服务器收到聊天消息信号
};

#endif // TCPMGR_H
