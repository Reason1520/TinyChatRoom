#include "chatpage.h"
#include "ui_chatpage.h"
#include <QStyleOption>
#include <QPainter>
#include "chatitembase.h"
#include "textbubble.h"
#include "picturebubble.h"
#include "applyfrienditem.h"
#include "usermgr.h"
#include <QJsonArray>
#include <QJsonObject>
#include "tcpmgr.h"
#include <QUuid>
#include <QStandardPaths>
#include "filetcpmgr.h"
#include <memory>

/******************************************************************************
 * @file       chatpage.h
 * @brief      聊天对话框的聊天部件类实现
 *
 * @author     lueying
 * @date       2026/1/21
 * @history
 *****************************************************************************/

ChatPage::ChatPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ChatPage) {
    ui->setupUi(this);
    //设置按钮样式
    ui->receive_btn->setState("normal", "hover", "press");
    ui->send_btn->setState("normal", "hover", "press");

    //设置图标样式
    ui->emo_lb->setState("normal", "hover", "press", "normal", "hover", "press");
    ui->file_lb->setState("normal", "hover", "press", "normal", "hover", "press");

}

ChatPage::~ChatPage() {
    delete ui;
}

void ChatPage::setChatData(std::shared_ptr<ChatThreadData> chat_data) {
    chat_data_ = chat_data;
    auto other_id = chat_data_->GetOtherId();
    if (other_id == 0) {
        //说明是群聊
        ui->title_lb->setText(chat_data_->GetGroupName());
        //todo...加载群聊信息和成员信息
        return;
    }

    //私聊
    auto friend_info = UserMgr::getInstance()->getFriendById(other_id);
    if (friend_info == nullptr) {
        return;
    }
    ui->title_lb->setText(friend_info->_name);
    ui->chat_data_list->removeAllItem();
    unrsp_item_map_.clear();
    base_item_map_.clear();
    for (auto& msg : chat_data->GetMsgMapRef()) {
        appendChatMsg(msg);
    }

    for (auto& msg : chat_data->GetMsgUnRspRef()) {
        appendChatMsg(msg, false);
    }
}

void ChatPage::appendChatMsg(std::shared_ptr<ChatDataBase> msg, bool rsp) {
    auto self_info = UserMgr::getInstance()->getUserInfo();
    ChatRole role;
    if (msg->GetSendUid() == self_info->_uid) {
        role = ChatRole::Self;
        ChatItemBase* pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(self_info->_name);
        setSelfIcon(pChatItem, self_info->_icon);
        QWidget* pBubble = nullptr;
        if (msg->GetMsgType() == ChatMsgType::TEXT) {
            pBubble = new TextBubble(role, msg->GetMsgContent());
        }
        else if (msg->GetMsgType() == ChatMsgType::PIC) {
            auto img_msg = std::dynamic_pointer_cast<ImgChatData>(msg);
            auto pic_bubble = new PictureBubble(img_msg->_msg_info->_preview_pix, role, img_msg->_msg_info->_total_size);
            pic_bubble->setState(img_msg->_msg_info->_transfer_state);
            pBubble = pic_bubble;
        }

        pChatItem->setWidget(pBubble);
        auto status = msg->GetStatus();
        pChatItem->setStatus(status);
        ui->chat_data_list->appendChatItem(pChatItem);
        if (rsp) {
            base_item_map_[msg->GetMsgId()] = pChatItem;
        }
        else {
            unrsp_item_map_[msg->GetUniqueId()] = pChatItem;
        }

    }
    else {
        role = ChatRole::Other;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::getInstance()->getFriendById(msg->GetSendUid());
        if (friend_info == nullptr) {
            return;
        }

        pChatItem->setUserName(friend_info->_name);

        // 使用正则表达式检查是否是默认头像
        QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
        QRegularExpressionMatch match = regex.match(friend_info->_icon);
        if (match.hasMatch()) {
            pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        }
        else {
            // 如果是用户上传的头像，获取存储目录
            QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            auto uid = UserMgr::getInstance()->getUid();
            QDir avatarsDir(storageDir + "/user/" + QString::number(msg->GetSendUid()) + "/avatars");
            // 确保目录存在
            if (avatarsDir.exists()) {
                QString avatarPath = avatarsDir.filePath(friend_info->_icon); // 获取上传头像的完整路径
                QPixmap pixmap(avatarPath); // 加载上传的头像图片
                if (!pixmap.isNull()) {
                    pChatItem->setUserIcon(pixmap);
                }
                else {
                    qWarning() << "无法加载上传的头像：" << avatarPath;
                    auto icon_label = pChatItem->getIconLabel();
                    loadHeadIcon(avatarPath, icon_label, friend_info->_icon, "other_icon");
                }
            }
            else {
                qWarning() << "头像存储目录不存在：" << avatarsDir.path();
                //创建目录
                avatarsDir.mkpath(".");
                auto icon_label = pChatItem->getIconLabel();
                QString avatarPath = avatarsDir.filePath(friend_info->_icon);
                loadHeadIcon(avatarPath, icon_label, friend_info->_icon, "other_icon");
            }
        }

        QWidget* pBubble = nullptr;
        if (msg->GetMsgType() == ChatMsgType::TEXT) {
            pBubble = new TextBubble(role, msg->GetMsgContent());
        }
        else if (msg->GetMsgType() == ChatMsgType::PIC) {
            auto img_msg = std::dynamic_pointer_cast<ImgChatData>(msg);
            auto pic_bubble = new PictureBubble(img_msg->_msg_info->_preview_pix, role, img_msg->_msg_info->_total_size);
            pic_bubble->setState(img_msg->_msg_info->_transfer_state);
            pBubble = pic_bubble;
        }
        pChatItem->setWidget(pBubble);
        auto status = msg->GetStatus();
        pChatItem->setStatus(status);
        ui->chat_data_list->appendChatItem(pChatItem);
        if (rsp) {
            base_item_map_[msg->GetMsgId()] = pChatItem;
        }
        else {
            unrsp_item_map_[msg->GetUniqueId()] = pChatItem;
        }
    }

}

void ChatPage::appendOtherMsg(std::shared_ptr<ChatDataBase> msg) {
    auto self_info = UserMgr::getInstance()->getUserInfo();
    ChatRole role;
    if (msg->GetSendUid() == self_info->_uid) {
        role = ChatRole::Self;
        ChatItemBase* pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(self_info->_name);
        setSelfIcon(pChatItem, self_info->_icon);
        QWidget* pBubble = nullptr;
        if (msg->GetMsgType() == ChatMsgType::TEXT) {
            pBubble = new TextBubble(role, msg->GetMsgContent());
        }
        else if (msg->GetMsgType() == ChatMsgType::PIC) {
            auto img_msg = std::dynamic_pointer_cast<ImgChatData>(msg);
            auto pic_bubble = new PictureBubble(img_msg->_msg_info->_preview_pix, role, img_msg->_msg_info->_total_size);
            pic_bubble->setMsgInfo(img_msg->_msg_info);
            pBubble = pic_bubble;
            //连接暂停和恢复信号
            connect(dynamic_cast<PictureBubble*>(pBubble), &PictureBubble::pauseRequested,
                this, &ChatPage::on_clicked_paused);
            connect(dynamic_cast<PictureBubble*>(pBubble), &PictureBubble::resumeRequested,
                this, &ChatPage::on_clicked_resume);
        }

        pChatItem->setWidget(pBubble);
        auto status = msg->GetStatus();
        pChatItem->setStatus(status);
        ui->chat_data_list->appendChatItem(pChatItem);
        base_item_map_[msg->GetMsgId()] = pChatItem;
    }
    else {
        role = ChatRole::Other;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::getInstance()->getFriendById(msg->GetSendUid());
        if (friend_info == nullptr) {
            return;
        }
        pChatItem->setUserName(friend_info->_name);

        // 使用正则表达式检查是否是默认头像
        QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
        QRegularExpressionMatch match = regex.match(friend_info->_icon);
        if (match.hasMatch()) {
            pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        } else {
            // 如果是用户上传的头像，获取存储目录
            QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir avatarsDir(storageDir + "/user/" + QString::number(msg->GetSendUid()) + "/avatars");
            // 确保目录存在
            if (avatarsDir.exists()) {
                QString avatarPath = avatarsDir.filePath(friend_info->_icon); // 获取上传头像的完整路径
                QPixmap pixmap(avatarPath); // 加载上传的头像图片
                if (!pixmap.isNull()) {
                    pChatItem->setUserIcon(pixmap);
                }
                else {
                    qWarning() << "无法加载上传的头像：" << avatarPath;
                    auto icon_label = pChatItem->getIconLabel();
                    loadHeadIcon(avatarPath, icon_label, friend_info->_icon, "other_icon");
                }
            }
            else {
                qWarning() << "头像存储目录不存在：" << avatarsDir.path();
                //创建目录
                avatarsDir.mkpath(".");
                auto icon_label = pChatItem->getIconLabel();
                QString avatarPath = avatarsDir.filePath(friend_info->_icon);
                loadHeadIcon(avatarPath, icon_label, friend_info->_icon, "other_icon");
            }
        }

        QWidget* pBubble = nullptr;
        if (msg->GetMsgType() == ChatMsgType::TEXT) {
            pBubble = new TextBubble(role, msg->GetMsgContent());
        }
        else if (msg->GetMsgType() == ChatMsgType::PIC) {
            auto img_msg = std::dynamic_pointer_cast<ImgChatData>(msg);
            auto pic_bubble = new PictureBubble(img_msg->_msg_info->_preview_pix, role, img_msg->_msg_info->_total_size);
            pic_bubble->setMsgInfo(img_msg->_msg_info);
            pBubble = pic_bubble;
            //连接暂停和恢复信号
            connect(dynamic_cast<PictureBubble*>(pBubble), &PictureBubble::pauseRequested,
                this, &ChatPage::on_clicked_paused);
            connect(dynamic_cast<PictureBubble*>(pBubble), &PictureBubble::resumeRequested,
                this, &ChatPage::on_clicked_resume);
        }
        pChatItem->setWidget(pBubble);
        auto status = msg->GetStatus();
        pChatItem->setStatus(status);
        ui->chat_data_list->appendChatItem(pChatItem);
        base_item_map_[msg->GetMsgId()] = pChatItem;
    }
}

void ChatPage::loadHeadIcon(QString avatarPath, QLabel* icon_label, QString file_name, QString req_type) {
    UserMgr::getInstance()->addLabelToReset(avatarPath, icon_label);
    //先加载默认的
    QPixmap pixmap(":/res/head_1.jpg");
    QPixmap scaledPixmap = pixmap.scaled(icon_label->size(),
        Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
    icon_label->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    icon_label->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

    //判断是否正在下载
    bool is_loading = UserMgr::getInstance()->isDownLoading(file_name);
    if (is_loading) {
        qWarning() << "正在下载: " << file_name;
    }
    else {
        //发送请求获取资源
        auto download_info = std::make_shared<DownloadInfo>();
        download_info->_name = file_name;
        download_info->_current_size = 0;
        download_info->_seq = 1;
        download_info->_total_size = 0;
        download_info->_client_path = avatarPath;
        //添加文件到管理者
        UserMgr::getInstance()->addDownloadFile(file_name, download_info);
        //发送消息
        FileTcpMgr::getInstance()->SendDownloadInfo(download_info, req_type);
    }
}

void ChatPage::updateChatStatus(std::shared_ptr<ChatDataBase> msg)
{
    auto iter = unrsp_item_map_.find(msg->GetUniqueId());
    //没找到则直接返回
    if (iter == unrsp_item_map_.end()) {
        return;
    }

    iter.value()->setStatus(msg->GetStatus());
    base_item_map_[msg->GetMsgId()] = iter.value();
    unrsp_item_map_.erase(iter);
}

void ChatPage::updateImgChatStatus(std::shared_ptr<ImgChatData> msg) {
    auto iter = unrsp_item_map_.find(msg->GetUniqueId());
    //没找到则直接返回
    if (iter == unrsp_item_map_.end()) {
        return;
    }

    iter.value()->setStatus(msg->GetStatus());
    base_item_map_[msg->GetMsgId()] = iter.value();
    unrsp_item_map_.erase(iter);

    auto bubble = base_item_map_[msg->GetMsgId()]->getBubble();
    PictureBubble* pic_bubble = dynamic_cast<PictureBubble*>(bubble);
    pic_bubble->setMsgInfo(msg->_msg_info);
}

void ChatPage::updateFileProgress(std::shared_ptr<MsgInfo> msg_info) {
    auto iter = base_item_map_.find(msg_info->_msg_id);
    if (iter == base_item_map_.end()) {
        return;
    }

    if (msg_info->_msg_type == MsgType::IMG_MSG) {
        auto bubble = iter.value()->getBubble();
        PictureBubble* pic_bubble = dynamic_cast<PictureBubble*>(bubble);
        pic_bubble->setProgress(msg_info->_rsp_size, msg_info->_total_size);
    }

}

void ChatPage::downloadFileFinished(std::shared_ptr<MsgInfo> msg_info, QString file_path) {
    auto iter = base_item_map_.find(msg_info->_msg_id);
    if (iter == base_item_map_.end()) {
        return;
    }

    if (msg_info->_msg_type == MsgType::IMG_MSG) {
        auto bubble = iter.value()->getBubble();
        PictureBubble* pic_bubble = dynamic_cast<PictureBubble*>(bubble);
        pic_bubble->setDownloadFinish(msg_info, file_path);
    }
}

void ChatPage::setSelfIcon(ChatItemBase* pChatItem, QString icon) {
    // 使用正则表达式检查是否是默认头像
    QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regex.match(icon);
    if (match.hasMatch()) {
        pChatItem->setUserIcon(QPixmap(icon));
    }
    else {
        // 如果是用户上传的头像，获取存储目录
        QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        auto uid = UserMgr::getInstance()->getUid();
        QDir avatarsDir(storageDir + "/user/" + QString::number(uid) + "/avatars");
        auto file_name = QFileInfo(icon).fileName();
        // 确保目录存在
        if (avatarsDir.exists()) {
            QString avatarPath = avatarsDir.filePath(file_name); // 获取上传头像的完整路径
            QPixmap pixmap(avatarPath); // 加载上传的头像图片
            if (!pixmap.isNull()) {
                pChatItem->setUserIcon(pixmap);
            }
            else {
                qWarning() << "无法加载上传的头像：" << avatarPath;
                auto icon_label = pChatItem->getIconLabel();
                loadHeadIcon(avatarPath, icon_label, file_name, "self_icon");
            }
        }
        else {
            qWarning() << "头像存储目录不存在：" << avatarsDir.path();
            //创建目录
            avatarsDir.mkpath(".");
            auto icon_label = pChatItem->getIconLabel();
            QString avatarPath = avatarsDir.filePath(file_name);
            loadHeadIcon(avatarPath, icon_label, file_name, "self_icon");
        }
    }
}

void ChatPage::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked() {
    if (chat_data_ == nullptr) {
        qDebug() << "friend_info is empty";
        return;
    }

    auto user_info = UserMgr::getInstance()->getUserInfo();
    auto pTextEdit = ui->chatEdit;
    ChatRole role = ChatRole::Self;
    QString userName = user_info->_name;
    QString userIcon = user_info->_icon;

    const QVector<std::shared_ptr<MsgInfo>>& msgList = pTextEdit->getMsgList();
    QJsonObject textObj;
    QJsonArray textArray;
    int txt_size = 0;
    auto thread_id = chat_data_->GetThreadId();
    for (int i = 0; i < msgList.size(); ++i) {
        //消息内容长度不合规就跳过
        if (msgList[i]->_text_or_url.length() > 1024) {
            continue;
        }

        MsgType type = msgList[i]->_msg_type;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        setSelfIcon(pChatItem, user_info->_icon);
        QWidget* pBubble = nullptr;
        //生成唯一id
        QUuid uuid = QUuid::createUuid();
        //转为字符串
        QString uuidString = uuid.toString();
        if (type == MsgType::TEXT_MSG) {
            pBubble = new TextBubble(role, msgList[i]->_text_or_url);
            if (txt_size + msgList[i]->_text_or_url.length() > 1024) {
                textObj["fromuid"] = user_info->_uid;
                textObj["touid"] = chat_data_->GetOtherId();
                textObj["thread_id"] = thread_id;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textObj);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                //发送并清空之前累计的文本列表
                txt_size = 0;
                textArray = QJsonArray();
                textObj = QJsonObject();
                //发送tcp请求给chat server
                emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
            }

            //将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
            //_bubble_map[uuidString] = pBubble;
            txt_size += msgList[i]->_text_or_url.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i]->_text_or_url.toUtf8();
            auto content = QString::fromUtf8(utf8Message);
            obj["content"] = content;
            obj["unique_id"] = uuidString;
            textArray.append(obj);
            //注意，此处先按私聊处理
            auto txt_msg = std::make_shared<TextChatData>(uuidString, thread_id, ChatFormType::PRIVATE,
                ChatMsgType::TEXT, content, user_info->_uid, 0);
            //将未回复的消息加入到未回复列表中，以便后续处理
            chat_data_->AppendUnRspMsg(uuidString, txt_msg);
        }
        else if (type == MsgType::IMG_MSG) {
            //将之前缓存的文本发送过去
            if (txt_size) {
                textObj["fromuid"] = user_info->_uid;
                textObj["touid"] = chat_data_->GetOtherId();
                textObj["thread_id"] = thread_id;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textObj);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                //发送并清空之前累计的文本列表
                txt_size = 0;
                textArray = QJsonArray();
                textObj = QJsonObject();
                //发送tcp请求给chat server
                emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
            }

            pBubble = new PictureBubble(QPixmap(msgList[i]->_text_or_url), role, msgList[i]->_total_size);
            //需要组织成文件发送，具体参考头像上传
            auto img_msg = std::make_shared<ImgChatData>(msgList[i], uuidString, thread_id, ChatFormType::PRIVATE,
                ChatMsgType::PIC, user_info->_uid, 0);
            //将未回复的消息加入到未回复列表中，以便后续处理
            chat_data_->AppendUnRspMsg(uuidString, img_msg);
            textObj["fromuid"] = user_info->_uid;
            textObj["touid"] = chat_data_->GetOtherId();
            textObj["thread_id"] = thread_id;
            textObj["md5"] = msgList[i]->_md5;
            textObj["name"] = msgList[i]->_unique_name;
            textObj["token"] = UserMgr::getInstance()->getToken();
            textObj["unique_id"] = uuidString;
            textObj["text_or_url"] = msgList[i]->_text_or_url;

            //文件信息加入管理
            UserMgr::getInstance()->addTransFile(msgList[i]->_unique_name, msgList[i]);
            QJsonDocument doc(textObj);
            QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
            //发送tcp请求给chat server
            emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_IMG_CHAT_MSG_REQ, jsonData);
            //链接暂停信号
            connect(dynamic_cast<PictureBubble*>(pBubble), &PictureBubble::pauseRequested,
                this, &ChatPage::on_clicked_paused);
            //链接恢复信号
            connect(dynamic_cast<PictureBubble*>(pBubble), &PictureBubble::resumeRequested,
                this, &ChatPage::on_clicked_resume);

        }
        else if (type == MsgType::FILE_MSG)
        {

        }
        //发送消息
        if (pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            // 设置聊天文本状态为未回复
            pChatItem->setStatus(0);
            ui->chat_data_list->appendChatItem(pChatItem);
            unrsp_item_map_[uuidString] = pChatItem;
        }

    }

    if (txt_size > 0) {
        qDebug() << "textArray is " << textArray;
        //发送给服务器
        textObj["text_array"] = textArray;
        textObj["fromuid"] = user_info->_uid;
        textObj["touid"] = chat_data_->GetOtherId();
        textObj["thread_id"] = thread_id;
        QJsonDocument doc(textObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
        //发送并清空之前累计的文本列表
        txt_size = 0;
        textArray = QJsonArray();
        textObj = QJsonObject();
        //发送tcp请求给chat server
        emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
    }
}


void ChatPage::on_receive_btn_clicked() {
    auto pTextEdit = ui->chatEdit;
    ChatRole role = ChatRole::Other;
    auto friend_info = UserMgr::getInstance()->getFriendById(chat_data_->GetOtherId());
    QString userName = friend_info->_name;
    QString userIcon = friend_info->_icon;

    const QVector<std::shared_ptr<MsgInfo>>& msgList = pTextEdit->getMsgList();
    for (int i = 0; i < msgList.size(); ++i)
    {
        MsgType type = msgList[i]->_msg_type;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget* pBubble = nullptr;
        if (type == MsgType::TEXT_MSG)
        {
            pBubble = new TextBubble(role, msgList[i]->_text_or_url);
        }
        else if (type == MsgType::IMG_MSG)
        {
            pBubble = new PictureBubble(QPixmap(msgList[i]->_text_or_url), role, msgList[i]->_total_size);
        }
        else if (type == MsgType::FILE_MSG)
        {

        }
        if (pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            pChatItem->setStatus(2);
            ui->chat_data_list->appendChatItem(pChatItem);
        }
    }
}

void ChatPage::on_clicked_paused(QString unique_name, TransferType transfer_type) {
    UserMgr::getInstance()->pauseTransFileByName(unique_name);
}

void ChatPage::on_clicked_resume(QString unique_name, TransferType transfer_type) {
    UserMgr::getInstance()->resumeTransFileByName(unique_name);
    //继续发送或者下载
    if (transfer_type == TransferType::Upload) {
        FileTcpMgr::getInstance()->ContinueUploadFile(unique_name);
        return;
    }

    if (transfer_type == TransferType::Download) {
        FileTcpMgr::getInstance()->ContinueDownloadFile(unique_name);
        return;
    }
}

void ChatPage::clearItems() {
    ui->chat_data_list->removeAllItem();
    unrsp_item_map_.clear();
    base_item_map_.clear();
}
