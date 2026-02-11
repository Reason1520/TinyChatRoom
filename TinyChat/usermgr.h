#ifndef USERMGR_H
#define USERMGR_H

#include <QObject>
#include <memory>
#include <singleton.h>
#include <QLabel>
#include "userdata.h"

/******************************************************************************
 * @file       usermgr.h
 * @brief      用户管理类
 *
 * @author     lueying
 * @date       2026/1/6
 * @history
 *****************************************************************************/

class UserMgr :public QObject, public Singleton<UserMgr>,
    public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~UserMgr();
    void setUserInfo(std::shared_ptr<UserInfo> user_info);  // 设置当前登录用户的个人基本信息
    void setToken(QString token);                           // 设置当前会话的登录 Token
    void setIcon(QString name);                             // 设置头像

    int getUid();
    QString getName();
    QString getNick();
    QString getIcon();
    QString getDesc();
    QString getToken();
    std::shared_ptr<UserInfo> getUserInfo();

    // 批量解析JSON数组并追加到好友申请列表中
    void appendApplyList(QJsonArray array);
    // 批量解析JSON数组并追加到好友列表，同时更新ID索引表
    void appendFriendList(QJsonArray array);
    // 获取当前所有的好友申请列表
    std::vector<std::shared_ptr<ApplyInfo>> getApplyList();
    // 向申请列表中手动添加一条新的申请记录
    void addApplyList(std::shared_ptr<ApplyInfo> app);
    // 检查指定UID是否已经在申请列表中
    bool alreadyApply(int uid);

    // 分页获取聊天列表中的好友数据
    std::vector<std::shared_ptr<UserInfo>> getChatListPerPage();
    // 判断聊天列表数据是否已全部加载完毕
    bool isLoadChatFin();
    // 更新聊天列表已加载的计数器，记录当前加载到的位置
    void updateChatLoadedCount();
    // 分页获取联系人列表中的好友数据
    std::vector<std::shared_ptr<UserInfo>> getConListPerPage();
    // 更新联系人列表已加载的计数器
    void updateContactLoadedCount();
    // 判断联系人列表数据是否已全部加载完毕
    bool isLoadConFin();

    // 根据UID检查指定用户是否是自己的好友
    bool checkFriendById(int uid);
    // 将认证响应中的信息转换为好友并存入索引 map
    void addFriend(std::shared_ptr<AuthRsp> auth_rsp);
    void addFriend(std::shared_ptr<AuthInfo> auth_info);
    // 根据UID在好友索引表中查找并返回好友信息对象
    std::shared_ptr<UserInfo> getFriendById(int uid);
    // 获取最近一次激活或打开的聊天会话 ID
    int getLastChatThreadId();
    // 记录当前正在进行的或最后一次操作的聊天会话 ID
    void setLastChatThreadId(int id);
    // 将会话线程对象存入管理中心
    void addChatThreadData(std::shared_ptr<ChatThreadData> chat_thread_data, int other_uid);
    // 根据好友的 UID 查找对应的会话 ID
    int getThreadIdByUid(int uid);
    // 根据会话 ID 直接获取该会话的详细数据对象
    std::shared_ptr<ChatThreadData> getChatThreadByThreadId(int thread_id);
    // 先根据好友 UID 找到会话 ID，再通过会话 ID 返回完整的会话数据对象
    std::shared_ptr<ChatThreadData> getChatThreadByUid(int uid);

    //获取当前正在加载的聊天数据。
    std::shared_ptr<ChatThreadData> getCurLoadData();
    std::shared_ptr<ChatThreadData> getNextLoadData();

    //将md5和文件信息关联起来
    void addUploadFile(QString name, std::shared_ptr<QFileInfo> file_info);
    //移除上传的文件信息
    void rmvUploadFile(QString name);
    //获取上传信息
    std::shared_ptr<QFileInfo> getUploadInfoByName(QString name);
    bool isDownLoading(QString name);
    void addDownloadFile(QString name, std::shared_ptr<DownloadInfo> file_info);
    void rmvDownloadFile(QString name);
    std::shared_ptr<DownloadInfo> getDownloadInfo(QString name);
    //添加资源路径到将要重置的Label集合
    void addLabelToReset(QString path, QLabel* label);
    void resetLabelIcon(QString path);
    void addTransFile(QString name, std::shared_ptr<MsgInfo> msg_info);
    std::shared_ptr<MsgInfo> getTransFileByName(QString name);
    void rmvTransFileByName(QString name);
    std::shared_ptr<MsgInfo> getFreeUploadFile();
    std::shared_ptr<MsgInfo> getFreeDownloadFile();
    void pauseTransFileByName(QString name);
    void resumeTransFileByName(QString name);
    bool transFileIsUploading(QString name);

public slots:
    // 处理添加好友成功的响应槽函数
    void slotAddFriendRsp(std::shared_ptr<AuthRsp> rsp);
    // 处理收到好友认证通知的槽函数
    void slotAddFriendAuth(std::shared_ptr<AuthInfo> auth);

private:
    UserMgr();
    std::shared_ptr<UserInfo> user_info_;
    QString token_;
    int uid_;
    std::vector<std::shared_ptr<ApplyInfo>> apply_list_;
    std::vector<std::shared_ptr<UserInfo>> friend_list_;
    QMap<int, std::shared_ptr<UserInfo>> friend_map_;
    int chat_loaded_;       // 聊天列表已加载的好友数量
    int contact_loaded_;    // 联系人列表已加载的好友数量
    //建立会话id到数据的映射关系
    QMap<int, std::shared_ptr<ChatThreadData>> chat_map_;
    //聊天会话id列表
    std::vector<int> chat_thread_ids_;
    //记录已经加载聊天列表的会话索引
    int cur_load_chat_index_;
    //上次会话的id
    int last_chat_thread_id_;
    //缓存其他用户uid和聊天的thread_id的映射关系。
    QMap<int, int> uid_to_thread_id_;
    std::mutex mtx_;

    //上传文件md5和文件信息关联 映射
    QMap<QString, std::shared_ptr<QFileInfo> > name_to_upload_info_;
    std::mutex down_load_mtx_;
    //名字关联下载信息
    QMap<QString, std::shared_ptr<DownloadInfo> > name_to_download_info_;
    QHash<QString, QList<QLabel*>> path_to_reset_labels_;
    //聊天传输文件映射
    QHash<QString, std::shared_ptr<MsgInfo> > name_to_msg_info_;
    //传输文件用的锁
    std::mutex trans_mtx_;
};

#endif // USERMGR_H

