#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include <QLabel>
#include "listitembase.h"
#include "userdata.h"

/******************************************************************************
 * @file       chatuserwid.h
 * @brief      聊天列表的用户构件
 *
 * @author     lueying
 * @date       2026/1/18
 * @history
 *****************************************************************************/

namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget* parent = nullptr);
    ~ChatUserWid();
    QSize sizeHint() const override;
    void setChatData(std::shared_ptr<ChatThreadData> chat_data);
    std::shared_ptr<ChatThreadData> getChatData();
    void showRedPoint(bool bshow);
    void updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);
    void loadHeadIcon(QString avatarPath, QLabel* icon_label, QString file_name, QString req_type);
private:
    Ui::ChatUserWid* ui;
    std::shared_ptr<ChatThreadData> chat_data_;
};

#endif // CHATUSERWID_H
