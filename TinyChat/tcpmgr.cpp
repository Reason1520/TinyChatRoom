#include "tcpmgr.h"
#include "usermgr.h"

/******************************************************************************
 * @file       tcp.cpp
 * @brief      服务器和客户端长连接TCP管理类实现
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

TcpThread::TcpThread() {
    tcp_thread_ = new QThread();
    // 将 TcpMgr 单例对象及其所有的槽函数（Slots）执行环境移动到了 tcp_thread_ 中
    TcpMgr::getInstance()->moveToThread(tcp_thread_);
    QObject::connect(tcp_thread_, &QThread::finished, tcp_thread_, &QObject::deleteLater);

    tcp_thread_->start();
}

TcpThread::~TcpThread() {
    tcp_thread_->quit();
}

TcpMgr::TcpMgr() :host_(""), port_(0), b_recv_pending_(false), message_id_(0), 
message_len_(0), bytes_sent_(0), pending_(false), socket_(this) {
    registerMetaType();
    QObject::connect(&socket_, &QTcpSocket::connected, [&]() {
        qDebug() << "Connected to server!";
        // 连接建立后发送消息
        emit sig_con_success(true);
    });

    // 读取数据
    QObject::connect(&socket_, &QTcpSocket::readyRead, this, [&]() {
        // 当有数据可读时，读取所有数据
        // 读取所有数据并追加到缓冲区
        buffer_.append(socket_.readAll());

        forever{
            //先解析头部
           if (!b_recv_pending_) {
               // 检查缓冲区中的数据是否足够解析出一个消息头（消息ID + 消息长度）
               if (buffer_.size() < static_cast<int>(sizeof(quint16) * 2)) {
                   return; // 数据不够，等待更多数据
               }

               // ✅ 每次都重新创建stream
               QDataStream stream(buffer_);
               stream.setVersion(QDataStream::Qt_5_0);
               stream >> message_id_ >> message_len_;
               buffer_.remove(0, sizeof(quint16) * 2);  // 使用remove代替mid赋值
               qDebug() << "Message ID:" << message_id_ << ", Length:" << message_len_;

           }

        //buffer剩余长读是否满足消息体长度，不满足则退出继续等待接受
        if (buffer_.size() < message_len_) {
            b_recv_pending_ = true;
            return;
        }

        b_recv_pending_ = false;
        // 读取消息体
        QByteArray messageBody = buffer_.mid(0, message_len_);
        qDebug() << "receive body msg is " << messageBody;

        buffer_ = buffer_.mid(message_len_);
        handleMsg(ReqId(message_id_),message_len_, messageBody);
        }

    });

    QObject::connect(&socket_, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
           Q_UNUSED(socketError)
           qDebug() << "Error:" << socket_.errorString();
    });

    // 处理连接断开
    QObject::connect(&socket_, &QTcpSocket::disconnected, [&]() {
        qDebug() << "Disconnected from server.";
        //并且发送通知到界面
        emit sig_connection_closed();
       });

    // 发送数据
    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);

    // 连接发送信号
    QObject::connect(&socket_, &QTcpSocket::bytesWritten, this, [this](qint64 bytes) {
        //更新发送数据
        bytes_sent_ += bytes;
        //未发送完整
        if (bytes_sent_ < current_block_.size()) {
            //继续发送
            auto data_to_send = current_block_.mid(bytes_sent_);
            socket_.write(data_to_send);
            return;
        }

        //发送完全，则查看队列是否为空
        if (send_queue_.isEmpty()) {
            //队列为空，说明已经将所有数据发送完成，将pending设置为false，这样后续要发送数据时可以继续发送
            current_block_.clear();
            pending_ = false;
            bytes_sent_ = 0;
            return;
        }

        //队列不为空，则取出队首元素，继续发送数据
        current_block_ = send_queue_.dequeue();
        bytes_sent_ = 0;
        pending_ = true;
        qint64 w2 = socket_.write(current_block_);
        qDebug() << "[TcpMgr] Dequeued and write() returned" << w2;
        });

    //关闭socket
    connect(this, &TcpMgr::sig_close, this, &TcpMgr::slot_tcp_close);
    // 注册消息
    initHandlers();
}

void TcpMgr::registerMetaType() {
    // 注册所有自定义类型
    qRegisterMetaType<ServerInfo>("ServerInfo");
    qRegisterMetaType<SearchInfo>("SearchInfo");
    qRegisterMetaType<std::shared_ptr<SearchInfo>>("std::shared_ptr<SearchInfo>");

    qRegisterMetaType<AddFriendApply>("AddFriendApply");
    qRegisterMetaType<std::shared_ptr<AddFriendApply>>("std::shared_ptr<AddFriendApply>");

    qRegisterMetaType<ApplyInfo>("ApplyInfo");

    qRegisterMetaType<std::shared_ptr<AuthInfo>>("std::shared_ptr<AuthInfo>");

    qRegisterMetaType<AuthRsp>("AuthRsp");
    qRegisterMetaType<std::shared_ptr<AuthRsp>>("std::shared_ptr<AuthRsp>");

    qRegisterMetaType<UserInfo>("UserInfo");

    qRegisterMetaType<std::vector<std::shared_ptr<TextChatData>>>("std::vector<std::shared_ptr<TextChatData>>");

    qRegisterMetaType<std::vector<std::shared_ptr<ChatThreadInfo>>>("std::vector<std::shared_ptr<ChatThreadInfo>>");

    qRegisterMetaType<std::shared_ptr<ChatThreadData>>("std::shared_ptr<ChatThreadData>");
    qRegisterMetaType<ReqId>("ReqId");
    qRegisterMetaType<std::shared_ptr<ImgChatData>>("std::shared_ptr<ImgChatData>");
    qRegisterMetaType<std::vector<std::shared_ptr<ChatDataBase>>>("std::vector<std::shared_ptr<ChatDataBase>>");
}

// 连接对端服务器
void TcpMgr::slot_tcp_connect(std::shared_ptr<ServerInfo> si) {
    qDebug() << "receive tcp connect signal";
    // 尝试连接到服务器
    qDebug() << "Connecting to server...";
    host_ = si->_chat_host;
    port_ = static_cast<uint16_t>(si->_chat_port.toUInt());
    socket_.connectToHost(host_, port_);
}

// 发送数据
void TcpMgr::slot_send_data(ReqId reqId, QByteArray data) {
    uint16_t id = reqId;

    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(data.length());

    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << id << len;

    // 添加字符串数据
    block.append(data);

    //判断是否正在发送
    if (pending_) {
        //放入队列直接返回，因为目前有数据正在发送
        send_queue_.enqueue(block);
        return;
    }

    // 没有正在发送，把这包设为“当前块”，重置计数，并写出去
    current_block_ = block;        // ← 保存当前正在发送的 block
    bytes_sent_ = 0;            // ← 归零
    pending_ = true;         // ← 标记正在发送

    qint64 written = socket_.write(current_block_);
    qDebug() << "tcp mgr send byte data is" << current_block_
        << ", write() returned" << written;
}

// 初始化消息处理器
void TcpMgr::initHandlers() {
    //auto self = shared_from_this();
    // 登录回调
    handlers_.insert(ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug() << "data jsonobj is " << jsonObj;

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err;
            emit sig_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Login Failed, err is " << err;
            emit sig_login_failed(err);
            return;
        }

        auto uid = jsonObj["uid"].toInt();
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto desc = jsonObj["desc"].toString();
        auto user_info = std::make_shared<UserInfo>(uid, name, nick, icon, sex, "", desc);

        UserMgr::getInstance()->setUserInfo(user_info);
        UserMgr::getInstance()->setToken(jsonObj["token"].toString());
        if (jsonObj.contains("apply_list")) {
            UserMgr::getInstance()->appendApplyList(jsonObj["apply_list"].toArray());
        }

        //添加好友列表
        if (jsonObj.contains("friend_list")) {
            UserMgr::getInstance()->appendFriendList(jsonObj["friend_list"].toArray());
        }

        emit sig_swich_chatdlg();
        });

    // 查找好友回调
    handlers_.insert(ID_SEARCH_USER_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err;
            emit sig_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Login Failed, err is " << err;
            emit sig_login_failed(err);
            return;
        }

        auto search_info = std::make_shared<SearchInfo>(jsonObj["uid"].toInt(),
            jsonObj["name"].toString(), jsonObj["nick"].toString(),
            jsonObj["desc"].toString(), jsonObj["sex"].toInt(), jsonObj["icon"].toString());

        emit sig_user_search(search_info);
        });

    // 收到好友申请回调函数
    handlers_.insert(ID_NOTIFY_ADD_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err;

            emit sig_user_search(nullptr);
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Login Failed, err is " << err;
            emit sig_user_search(nullptr);
            return;
        }

        int from_uid = jsonObj["applyuid"].toInt();
        QString name = jsonObj["name"].toString();
        QString desc = jsonObj["desc"].toString();
        QString icon = jsonObj["icon"].toString();
        QString nick = jsonObj["nick"].toString();
        int sex = jsonObj["sex"].toInt();

        auto apply_info = std::make_shared<AddFriendApply>(
            from_uid, name, desc,
            icon, nick, sex);

        emit sig_friend_apply(apply_info);
        });

    // 同意好友申请回调函数（同意者）
    handlers_.insert(ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }

        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto uid = jsonObj["uid"].toInt();

        std::vector<std::shared_ptr<TextChatData>> chat_datas;
        for (const QJsonValue& data : jsonObj["chat_datas"].toArray()) {
            auto send_uid = data["sender"].toInt();
            auto msg_id = data["msg_id"].toInt();
            auto thread_id = data["thread_id"].toInt();
            auto unique_id = data["unique_id"].toInt();
            auto msg_content = data["msg_content"].toString();
            auto status = data["status"].toInt();
            auto chat_data = std::make_shared<TextChatData>(msg_id, thread_id, ChatFormType::PRIVATE,
                ChatMsgType::TEXT, msg_content, send_uid, status);
            chat_datas.push_back(chat_data);
        }

        auto rsp = std::make_shared<AuthRsp>(uid, name, nick, icon, sex);
        rsp->SetChatDatas(chat_datas);
        emit sig_auth_rsp(rsp);

        qDebug() << "Auth Friend Success ";
        });

    // 收到同意好友请求回调函数
    handlers_.insert(ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }

        int from_uid = jsonObj["fromuid"].toInt();
        QString name = jsonObj["name"].toString();
        QString nick = jsonObj["nick"].toString();
        QString icon = jsonObj["icon"].toString();
        int sex = jsonObj["sex"].toInt();

        std::vector<std::shared_ptr<TextChatData>> chat_datas;
        for (const QJsonValue& data : jsonObj["chat_datas"].toArray()) {
            auto send_uid = data["sender"].toInt();
            auto msg_id = data["msg_id"].toInt();
            auto thread_id = data["thread_id"].toInt();
            auto unique_id = data["unique_id"].toInt();
            auto msg_content = data["msg_content"].toString();
            QString chat_time = data["chat_time"].toString();
            auto status = data["status"].toInt();
            auto chat_data = std::make_shared<TextChatData>(msg_id, thread_id, ChatFormType::PRIVATE,
                ChatMsgType::TEXT, msg_content, send_uid, status, chat_time);
            chat_datas.push_back(chat_data);
        }

        auto auth_info = std::make_shared<AuthInfo>(from_uid, name,
            nick, icon, sex);

        auth_info->SetChatDatas(chat_datas);

        emit sig_add_auth_friend(auth_info);
        });

    // 发送端发送聊天信息回调函数
    handlers_.insert(ID_TEXT_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Chat Msg Rsp Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Chat Msg Rsp Failed, err is " << err;
            return;
        }

        qDebug() << "Receive Text Chat Rsp Success ";
        //收到消息后转发给页面
        auto thread_id = jsonObj["thread_id"].toInt();
        auto sender = jsonObj["fromuid"].toInt();


        std::vector<std::shared_ptr<TextChatData>> chat_datas;
        for (const QJsonValue& data : jsonObj["chat_datas"].toArray()) {
            auto msg_id = data["message_id"].toInt();
            auto unique_id = data["unique_id"].toString();
            auto msg_content = data["content"].toString();
            QString chat_time = data["chat_time"].toString();
            int status = data["status"].toInt();
            auto chat_data = std::make_shared<TextChatData>(msg_id, unique_id, thread_id, ChatFormType::PRIVATE,
                ChatMsgType::TEXT, msg_content, sender, status, chat_time);
            chat_datas.push_back(chat_data);
        }

        //发送信号通知界面
        emit sig_chat_msg_rsp(thread_id, chat_datas);

        });

    // 接收端收到聊天消息回调函数
    handlers_.insert(ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Notify Chat Msg Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Notify Chat Msg Failed, err is " << err;
            return;
        }

        qDebug() << "Receive Text Chat Notify Success ";

        //收到消息后转发给页面
        auto thread_id = jsonObj["thread_id"].toInt();
        auto sender = jsonObj["fromuid"].toInt();


        std::vector<std::shared_ptr<TextChatData>> chat_datas;
        for (const QJsonValue& data : jsonObj["chat_datas"].toArray()) {
            auto msg_id = data["message_id"].toInt();
            auto unique_id = data["unique_id"].toString();
            auto msg_content = data["content"].toString();
            QString chat_time = data["chat_time"].toString();
            int status = data["status"].toInt();
            auto chat_data = std::make_shared<TextChatData>(msg_id, unique_id, thread_id, ChatFormType::PRIVATE,
                ChatMsgType::TEXT, msg_content, sender, status, chat_time);
            chat_datas.push_back(chat_data);
        }


        emit sig_text_chat_msg(chat_datas);
        });

    // 收到离线通知回调函数
    handlers_.insert(ID_NOTIFY_OFF_LINE_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Notify Chat Msg Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Notify Chat Msg Failed, err is " << err;
            return;
        }

        auto uid = jsonObj["uid"].toInt();
        qDebug() << "Receive offline Notify Success, uid is " << uid;
        //断开连接
        //并且发送通知到界面
        emit sig_notify_offline();

        });

    // 心跳回调函数
    handlers_.insert(ID_HEARTBEAT_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Heart Beat Msg Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Heart Beat Msg Failed, err is " << err;
            return;
        }

        qDebug() << "Receive Heart Beat Msg Success";

        });

    // 注册加载聊天线程列表回调函数
    handlers_.insert(ID_LOAD_CHAT_THREAD_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "chat thread json parse failed " << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "get chat thread rsp failed, error is " << err;
            return;
        }

        qDebug() << "Receive chat thread rsp Success";

        auto thread_array = jsonObj["threads"].toArray();
        std::vector<std::shared_ptr<ChatThreadInfo>> chat_threads;
        for (const QJsonValue& value : thread_array) {
            auto cti = std::make_shared<ChatThreadInfo>();
            cti->_thread_id = value["thread_id"].toInt();
            cti->_type = value["type"].toString();
            cti->_user1_id = value["user1_id"].toInt();
            cti->_user2_id = value["user2_id"].toInt();
            chat_threads.push_back(cti);
        }

        bool load_more = jsonObj["load_more"].toBool();
        int next_last_id = jsonObj["next_last_id"].toInt();
        //发送信号通知界面
        emit sig_load_chat_thread(load_more, next_last_id, chat_threads);
        });

    // 注册创建私聊回调函数
    handlers_.insert(ID_CREATE_PRIVATE_CHAT_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "parse create private chat json parse failed " << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "get create private chat failed, error is " << err;
            return;
        }

        qDebug() << "Receive create private chat rsp Success";

        int uid = jsonObj["uid"].toInt();
        int other_id = jsonObj["other_id"].toInt();
        int thread_id = jsonObj["thread_id"].toInt();

        //发送信号通知界面
        emit sig_create_private_chat(uid, other_id, thread_id);
        });

    // 加载聊天消息记录回调函数
    handlers_.insert(ID_LOAD_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "parse create private chat json parse failed " << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "get create private chat failed, error is " << err;
            return;
        }

        qDebug() << "Receive create private chat rsp Success";

        int thread_id = jsonObj["thread_id"].toInt();
        int last_msg_id = jsonObj["last_message_id"].toInt();
        bool load_more = jsonObj["load_more"].toBool();

        std::vector<std::shared_ptr<ChatDataBase>> chat_datas;
        for (const QJsonValue& data : jsonObj["chat_datas"].toArray()) {
            auto send_uid = data["sender"].toInt();
            auto message_id = data["message_id"].toInt();
            auto thread_id = data["thread_id"].toInt();
            auto unique_id = data["unique_id"].toInt();
            auto msg_content = data["msg_content"].toString();
            QString chat_time = data["chat_time"].toString();
            int status = data["status"].toInt();
            int msg_type = data["msg_type"].toInt();
            int recv_id = data["receiver"].toInt();
            auto chat_data = std::make_shared<ChatDataBase>(message_id, thread_id, ChatFormType::PRIVATE,
                ChatMsgType::TEXT, msg_content, send_uid, status, chat_time);
            chat_datas.push_back(chat_data);
        }

        //发送信号通知界面
        emit sig_load_chat_msg(thread_id, last_msg_id, load_more, chat_datas);
        });
}

// 处理消息
void TcpMgr::handleMsg(ReqId id, int len, QByteArray data) {
    auto find_iter = handlers_.find(id);
    if (find_iter == handlers_.end()) {
        qDebug() << "not found id [" << id << "] to handle";
        return;
    }

    find_iter.value()(id, len, data);
}

// 关闭链接
void TcpMgr::closeConnection() {
    emit sig_close();
}

// tcp关闭槽函数
void TcpMgr::slot_tcp_close() {
    socket_.close();
}