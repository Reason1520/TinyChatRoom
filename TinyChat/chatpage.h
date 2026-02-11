#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include <QMap>
#include "userdata.h"
#include "chatitembase.h"

/******************************************************************************
 * @file       chatpage.h
 * @brief      聊天对话框的聊天部件类
 *
 * @author     lueying
 * @date       2026/1/21
 * @history
 *****************************************************************************/

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT
public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
    // 设置当前页面的聊天数据
    void setChatData(std::shared_ptr<ChatThreadData> chat_data);
    // 将单条聊天消息封装成 ChatItem 并添加到界面列表中
    void appendChatMsg(std::shared_ptr<ChatDataBase> msg, bool rsp = true);
    // 更新普通消息（文本）的发送状态
    void updateChatStatus(std::shared_ptr<ChatDataBase> msg);
    // 更新图片消息的发送状态
    void updateImgChatStatus(std::shared_ptr<ImgChatData> img_msg);
    // 设置并加载自己的头像
    void setSelfIcon(ChatItemBase* pChatItem, QString icon);
    // 实时更新图片或文件的传输进度条
    void updateFileProgress(std::shared_ptr<MsgInfo> msg_info);
    // 异步加载头像逻辑，若本地不存在则调用 FileTcpMgr 发起下载请求
    void loadHeadIcon(QString avatarPath, QLabel* icon_label, QString file_name, QString req_type);
    // 专门用于处理接收到的他人消息
    void appendOtherMsg(std::shared_ptr<ChatDataBase> msg);
    // 处理文件/图片下载完成后的逻辑
    void downloadFileFinished(std::shared_ptr<MsgInfo> msg_info, QString file_path);

protected:
    void paintEvent(QPaintEvent *event);
private slots:
    void on_send_btn_clicked();
    void on_receive_btn_clicked();
    //接收PictureBubble传回来的暂停信号
    void on_clicked_paused(QString unique_name, TransferType transfer_type);
    //接收PictureBubble传回来的继续信号
    void on_clicked_resume(QString unique_name, TransferType transfer_type);

private:
    void clearItems();
    Ui::ChatPage *ui;
    std::shared_ptr<ChatThreadData> chat_data_;
    QMap<QString, QWidget*>  bubble_map_;
    //管理未回复聊天信息
    QHash<QString, ChatItemBase*> unrsp_item_map_;
    //管理已经回复的消息
    QHash<qint64, ChatItemBase*> base_item_map_;
signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
};

#endif // CHATPAGE_H
