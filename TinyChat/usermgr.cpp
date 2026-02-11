#include "usermgr.h"
#include <QJsonArray>
#include "tcpmgr.h"

/******************************************************************************
 * @file       usermgr.h
 * @brief      用户管理类实现
 *
 * @author     lueying
 * @date       2026/1/6
 * @history
 *****************************************************************************/

UserMgr::~UserMgr() {

}

void UserMgr::setUserInfo(std::shared_ptr<UserInfo> user_info) {
    std::lock_guard<std::mutex> lock(mtx_);
    user_info_ = user_info;
}

void UserMgr::setToken(QString token)
{
    std::lock_guard<std::mutex> lock(mtx_);
    token_ = token;
}

QString UserMgr::getToken() {
    std::lock_guard<std::mutex> lock(mtx_);
    return token_;
}

int UserMgr::getUid()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return user_info_->_uid;
}

QString UserMgr::getName()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return user_info_->_name;
}

QString UserMgr::getNick()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return user_info_->_nick;
}

QString UserMgr::getIcon()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return user_info_->_icon;
}

void UserMgr::setIcon(QString name) {
    std::lock_guard<std::mutex> lock(mtx_);
    user_info_->_icon = name;
}

QString UserMgr::getDesc()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return user_info_->_desc;
}

std::shared_ptr<UserInfo> UserMgr::getUserInfo()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return user_info_;
}


void UserMgr::appendApplyList(QJsonArray array) {
    // 遍历 QJsonArray 并输出每个元素
    for (const QJsonValue& value : array) {
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto sex = value["sex"].toInt();
        auto uid = value["uid"].toInt();
        auto status = value["status"].toInt();
        auto info = std::make_shared<ApplyInfo>(uid, name,
            desc, icon, nick, sex, status);
        std::lock_guard<std::mutex> lock(mtx_);
        apply_list_.push_back(info);
    }
}

void UserMgr::appendFriendList(QJsonArray array) {
    // 遍历 QJsonArray 并输出每个元素
    for (const QJsonValue& value : array) {
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto sex = value["sex"].toInt();
        auto uid = value["uid"].toInt();
        auto back = value["back"].toString();

        auto info = std::make_shared<UserInfo>(uid, name,
            nick, icon, sex, desc, back);
        std::lock_guard<std::mutex> lock(mtx_);
        friend_list_.push_back(info);
        friend_map_.insert(uid, info);
    }
}

std::vector<std::shared_ptr<ApplyInfo> > UserMgr::getApplyList() {
    std::lock_guard<std::mutex> lock(mtx_);
    return apply_list_;
}

void UserMgr::addApplyList(std::shared_ptr<ApplyInfo> app) {
    std::lock_guard<std::mutex> lock(mtx_);
    apply_list_.push_back(app);
}

bool UserMgr::alreadyApply(int uid) {
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& apply : apply_list_) {
        if (apply->_uid == uid) {
            return true;
        }
    }

    return false;
}

std::vector<std::shared_ptr<UserInfo>> UserMgr::getChatListPerPage() {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<std::shared_ptr<UserInfo>> friend_list;
    int begin = chat_loaded_;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= friend_list_.size()) {
        return friend_list;
    }

    if (end > friend_list_.size()) {
        friend_list = std::vector<std::shared_ptr<UserInfo>>(friend_list_.begin() + begin, friend_list_.end());
        return friend_list;
    }


    friend_list = std::vector<std::shared_ptr<UserInfo>>(friend_list_.begin() + begin, friend_list_.begin() + end);
    return friend_list;
}


std::vector<std::shared_ptr<UserInfo>> UserMgr::getConListPerPage() {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<std::shared_ptr<UserInfo>> friend_list;
    int begin = contact_loaded_;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= friend_list_.size()) {
        return friend_list;
    }

    if (end > friend_list_.size()) {
        friend_list = std::vector<std::shared_ptr<UserInfo>>(friend_list_.begin() + begin, friend_list_.end());
        return friend_list;
    }


    friend_list = std::vector<std::shared_ptr<UserInfo>>(friend_list_.begin() + begin, friend_list_.begin() + end);
    return friend_list;
}

// 初始化成员变量和加载计数
UserMgr::UserMgr() :user_info_(nullptr), chat_loaded_(0), contact_loaded_(0), last_chat_thread_id_(0), cur_load_chat_index_(0) {

}

void UserMgr::slotAddFriendRsp(std::shared_ptr<AuthRsp> rsp) {
    addFriend(rsp);
}

void UserMgr::slotAddFriendAuth(std::shared_ptr<AuthInfo> auth) {
    addFriend(auth);
}

bool UserMgr::isLoadChatFin() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (chat_loaded_ >= friend_list_.size()) {
        return true;
    }

    return false;
}

void UserMgr::updateChatLoadedCount() {
    std::lock_guard<std::mutex> lock(mtx_);
    int begin = chat_loaded_;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= friend_list_.size()) {
        return;
    }

    if (end > friend_list_.size()) {
        chat_loaded_ = friend_list_.size();
        return;
    }

    chat_loaded_ = end;
}

void UserMgr::updateContactLoadedCount() {
    std::lock_guard<std::mutex> lock(mtx_);
    int begin = contact_loaded_;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= friend_list_.size()) {
        return;
    }

    if (end > friend_list_.size()) {
        contact_loaded_ = friend_list_.size();
        return;
    }

    contact_loaded_ = end;
}

bool UserMgr::isLoadConFin() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (contact_loaded_ >= friend_list_.size()) {
        return true;
    }

    return false;
}

bool UserMgr::checkFriendById(int uid) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto iter = friend_map_.find(uid);
    if (iter == friend_map_.end()) {
        return false;
    }

    return true;
}

void UserMgr::addFriend(std::shared_ptr<AuthRsp> auth_rsp) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto friend_info = std::make_shared<UserInfo>(auth_rsp);
    friend_map_[friend_info->_uid] = friend_info;
}

void UserMgr::addFriend(std::shared_ptr<AuthInfo> auth_info) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto friend_info = std::make_shared<UserInfo>(auth_info);
    friend_map_[friend_info->_uid] = friend_info;
}

std::shared_ptr<UserInfo> UserMgr::getFriendById(int uid) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto find_it = friend_map_.find(uid);
    if (find_it == friend_map_.end()) {
        return nullptr;
    }

    return *find_it;
}

int UserMgr::getLastChatThreadId() {
    std::lock_guard<std::mutex> lock(mtx_);
    return last_chat_thread_id_;
}

void UserMgr::setLastChatThreadId(int id) {
    std::lock_guard<std::mutex> lock(mtx_);
    last_chat_thread_id_ = id;
}

void UserMgr::addChatThreadData(std::shared_ptr<ChatThreadData> chat_thread_data, int other_uid) {
    std::lock_guard<std::mutex> lock(mtx_);
    //建立会话id到数据的映射关系
    chat_map_[chat_thread_data->GetThreadId()] = chat_thread_data;
    //存储会话列表
    chat_thread_ids_.push_back(chat_thread_data->GetThreadId());
    if (other_uid) {
        //将对方uid和会话id关联
        uid_to_thread_id_[other_uid] = chat_thread_data->GetThreadId();
    }
}

int UserMgr::getThreadIdByUid(int uid) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto iter = uid_to_thread_id_.find(uid);
    if (iter == uid_to_thread_id_.end()) {
        return -1;
    }

    return iter.value();
}

std::shared_ptr<ChatThreadData> UserMgr::getChatThreadByThreadId(int thread_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto find_iter = chat_map_.find(thread_id);
    if (find_iter != chat_map_.end()) {
        return find_iter.value();
    }
    return nullptr;
}

std::shared_ptr<ChatThreadData> UserMgr::getChatThreadByUid(int uid) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto iter = uid_to_thread_id_.find(uid);
    if (iter == uid_to_thread_id_.end()) {
        return nullptr;
    }

    auto chat_iter = chat_map_.find(iter.value());
    if (chat_iter == chat_map_.end()) {
        return nullptr;
    }

    return chat_iter.value();
}



std::shared_ptr<ChatThreadData> UserMgr::getCurLoadData() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (cur_load_chat_index_ >= chat_thread_ids_.size()) {
        return nullptr;
    }

    auto iter = chat_map_.find(chat_thread_ids_[cur_load_chat_index_]);
    if (iter == chat_map_.end()) {
        return nullptr;
    }

    return iter.value();
}

std::shared_ptr<ChatThreadData> UserMgr::getNextLoadData() {
    std::lock_guard<std::mutex> lock(mtx_);
    cur_load_chat_index_++;
    if (cur_load_chat_index_ >= chat_thread_ids_.size()) {
        return nullptr;
    }

    auto iter = chat_map_.find(chat_thread_ids_[cur_load_chat_index_]);
    if (iter == chat_map_.end()) {
        return nullptr;
    }

    return iter.value();
}



void UserMgr::addUploadFile(QString name, std::shared_ptr<QFileInfo> file_info) {
    std::lock_guard<std::mutex> lock(mtx_);
    name_to_upload_info_.insert(name, file_info);
}

void UserMgr::rmvUploadFile(QString name) {
    std::lock_guard<std::mutex> lock(mtx_);
    name_to_upload_info_.remove(name);
}

std::shared_ptr<QFileInfo> UserMgr::getUploadInfoByName(QString name) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto iter = name_to_upload_info_.find(name);
    if (iter == name_to_upload_info_.end()) {
        return nullptr;
    }

    return iter.value();
}


bool UserMgr::isDownLoading(QString name) {
    std::lock_guard<std::mutex> lock(down_load_mtx_);
    auto iter = name_to_download_info_.find(name);
    if (iter == name_to_download_info_.end()) {
        return false;
    }

    return true;
}

void UserMgr::addDownloadFile(QString name, std::shared_ptr<DownloadInfo> file_info) {
    std::lock_guard<std::mutex> lock(down_load_mtx_);
    name_to_download_info_[name] = file_info;
}

void UserMgr::rmvDownloadFile(QString name) {
    std::lock_guard<std::mutex> lock(down_load_mtx_);
    name_to_download_info_.remove(name);
}

std::shared_ptr<DownloadInfo> UserMgr::getDownloadInfo(QString name) {
    std::lock_guard<std::mutex> lock(down_load_mtx_);
    auto iter = name_to_download_info_.find(name);
    if (iter == name_to_download_info_.end()) {
        return nullptr;
    }

    return iter.value();
}


void UserMgr::addLabelToReset(QString path, QLabel* label) {
    auto iter = path_to_reset_labels_.find(path);
    if (iter == path_to_reset_labels_.end()) {
        QList<QLabel*> list;
        list.append(label);
        path_to_reset_labels_.insert(path, list);
        return;
    }

    iter->append(label);
}

void UserMgr::resetLabelIcon(QString path) {
    auto iter = path_to_reset_labels_.find(path);
    if (iter == path_to_reset_labels_.end()) {
        return;
    }

    for (auto ele_iter = iter.value().begin(); ele_iter != iter.value().end(); ele_iter++) {
        QPixmap pixmap(path); // 加载上传的头像图片
        if (!pixmap.isNull()) {
            QPixmap scaledPixmap = pixmap.scaled((*ele_iter)->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            (*ele_iter)->setPixmap(scaledPixmap);
            (*ele_iter)->setScaledContents(true);
        }
        else {
            qWarning() << "无法加载上传的头像：" << path;
        }
    }

    path_to_reset_labels_.erase(iter);
}

void UserMgr::addTransFile(QString name, std::shared_ptr<MsgInfo> msg_info) {
    std::lock_guard<std::mutex> mtx(trans_mtx_);
    name_to_msg_info_[name] = msg_info;
}

std::shared_ptr<MsgInfo> UserMgr::getTransFileByName(QString name) {
    std::lock_guard<std::mutex> mtx(trans_mtx_);
    auto iter = name_to_msg_info_.find(name);
    if (iter == name_to_msg_info_.end()) {
        return nullptr;
    }

    return *iter;
}

void UserMgr::rmvTransFileByName(QString name) {
    std::lock_guard<std::mutex> mtx(trans_mtx_);
    auto iter = name_to_msg_info_.find(name);
    if (iter == name_to_msg_info_.end()) {
        return;
    }

    name_to_msg_info_.erase(iter);
}

void UserMgr::pauseTransFileByName(QString name) {
    std::lock_guard<std::mutex> mtx(trans_mtx_);
    auto iter = name_to_msg_info_.find(name);
    if (iter == name_to_msg_info_.end()) {
        return;
    }

    iter.value()->_transfer_state = TransferState::Paused;
}

void UserMgr::resumeTransFileByName(QString name) {
    std::lock_guard<std::mutex> mtx(trans_mtx_);
    auto iter = name_to_msg_info_.find(name);
    if (iter == name_to_msg_info_.end()) {
        return;
    }

    if (iter.value()->_transfer_type == TransferType::Download) {
        iter.value()->_transfer_state = TransferState::Downloading;
        return;
    }

    if (iter.value()->_transfer_type == TransferType::Upload) {
        iter.value()->_transfer_state = TransferState::Uploading;
        return;
    }
}

bool UserMgr::transFileIsUploading(QString name) {
    std::lock_guard<std::mutex> mtx(trans_mtx_);
    auto iter = name_to_msg_info_.find(name);
    if (iter == name_to_msg_info_.end()) {
        return false;
    }

    if (iter.value()->_transfer_state == TransferState::Uploading) {
        return true;
    }

    return false;
}

std::shared_ptr<MsgInfo> UserMgr::getFreeUploadFile() {
    std::lock_guard<std::mutex> mtx(trans_mtx_);
    if (name_to_msg_info_.isEmpty()) {
        return nullptr;
    }

    for (auto iter = name_to_msg_info_.begin(); iter != name_to_msg_info_.end(); iter++) {
        //只要传输状态不是暂停则返回一个可用的待传输文件
        if ((iter.value()->_transfer_state != TransferState::Paused) &&
            (iter.value()->_transfer_type == TransferType::Upload)) {
            return iter.value();
        }
    }

    //没有找到等待传输的文件则返回空
    return nullptr;
}

std::shared_ptr<MsgInfo> UserMgr::getFreeDownloadFile() {
    std::lock_guard<std::mutex> mtx(trans_mtx_);
    if (name_to_msg_info_.isEmpty()) {
        return nullptr;
    }

    for (auto iter = name_to_msg_info_.begin(); iter != name_to_msg_info_.end(); iter++) {
        //只要传输状态不是暂停则返回一个可用的待传输文件
        if ((iter.value()->_transfer_state != TransferState::Paused) &&
            (iter.value()->_transfer_type == TransferType::Download)) {
            return iter.value();
        }
    }

    //没有找到等待传输的文件则返回空
    return nullptr;
}
