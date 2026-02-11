#include "chatuserwid.h"
#include "global.h"
#include "usermgr.h"
#include "ui_chatuserwid.h"
#include <QRegularExpression>
#include <QStandardPaths>
#include "filetcpmgr.h"

/******************************************************************************
 * @file       chatuserwid.cpp
 * @brief      聊天列表的用户构件实现
 *
 * @author     lueying
 * @date       2026/1/18
 * @history
 *****************************************************************************/

ChatUserWid::ChatUserWid(QWidget* parent) :
    ListItemBase(parent),
    ui(new Ui::ChatUserWid) {
    ui->setupUi(this);
    setItemType(ListItemType::ChatUserItem);
    ui->red_point->raise();
    showRedPoint(false);
}

ChatUserWid::~ChatUserWid()  {
    delete ui;
}

QSize ChatUserWid::sizeHint() const {
    return QSize(250, 70); // 返回自定义的尺寸
}

void ChatUserWid::setChatData(std::shared_ptr<ChatThreadData> chat_data) {
    chat_data_ = chat_data;
    auto other_id = chat_data_->GetOtherId();
    auto other_info = UserMgr::getInstance()->getFriendById(other_id);
    // 加载图片

    QString head_icon = UserMgr::getInstance()->getIcon();

    // 使用正则表达式检查是否是默认头像
    QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regex.match(other_info->_icon);

    if (match.hasMatch()) {
        // 如果是默认头像（:/res/head_X.jpg 格式）
        QPixmap pixmap(other_info->_icon); // 加载默认头像图片
        QPixmap scaledPixmap = pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->icon_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
        ui->icon_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
    }
    else {
        // 如果是用户上传的头像，获取存储目录
        QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        auto other_uid = other_info->_uid;
        QDir avatarsDir(storageDir + "/user/" + QString::number(other_uid) + "/avatars");

        // 确保目录存在
        if (avatarsDir.exists()) {
            QString avatarPath = avatarsDir.filePath(QFileInfo(other_info->_icon).fileName()); // 获取上传头像的完整路径
            QPixmap pixmap(avatarPath); // 加载上传的头像图片
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                ui->icon_lb->setPixmap(scaledPixmap);
                ui->icon_lb->setScaledContents(true);
            }
            else {
                qWarning() << "无法加载上传的头像：" << avatarPath;
                loadHeadIcon(avatarPath, ui->icon_lb, other_info->_icon, "other_icon");
            }
        }
        else {
            qWarning() << "头像存储目录不存在：" << avatarsDir.path();
            QString avatarPath = avatarsDir.filePath(QFileInfo(other_info->_icon).fileName());
            avatarsDir.mkpath(".");
            loadHeadIcon(avatarPath, ui->icon_lb, other_info->_icon, "other_icon");
        }
    }

    ui->user_name_lb->setText(other_info->_name);

    ui->user_chat_lb->setText(chat_data->GetLastMsg());
}

void ChatUserWid::loadHeadIcon(QString avatarPath, QLabel* icon_label, QString file_name, QString req_type) {
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

std::shared_ptr<ChatThreadData> ChatUserWid::getChatData() {
    return chat_data_;
}

void ChatUserWid::showRedPoint(bool bshow) {
    if (bshow) {
        ui->red_point->show();
    }
    else {
        ui->red_point->hide();
    }
}



void ChatUserWid::updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs) {

    int last_msg_id = 0;
    QString last_msg = "";
    for (auto& msg : msgs) {
        last_msg = msg->GetContent();
        last_msg_id = msg->GetMsgId();
        chat_data_->AddMsg(msg);
    }

    chat_data_->SetLastMsgId(last_msg_id);
    ui->user_chat_lb->setText(last_msg);
}


