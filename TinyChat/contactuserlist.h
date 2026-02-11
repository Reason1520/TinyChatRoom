#ifndef CONTACTUSERLIST_H
#define CONTACTUSERLIST_H
#include <QListWidget>
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>
#include <memory>
#include "userdata.h"

/******************************************************************************
 * @file       contactuserlist.h
 * @brief      联系人列表类
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

class ConUserItem;

class ContactUserList : public QListWidget
{
    Q_OBJECT
public:
    ContactUserList(QWidget* parent = nullptr);
    void showRedPoint(bool bshow = true);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override ;
private:
    void addContactUserList();

public slots:
     void slot_item_clicked(QListWidgetItem *item);
     //void slot_add_auth_firend(std::shared_ptr<AuthInfo>);
     // 收到同意好友申请槽函数
     void slot_auth_rsp(std::shared_ptr<AuthRsp>);
signals:
    void sig_loading_contact_user();
    void sig_switch_apply_friend_page();
    void sig_switch_friend_info_page(std::shared_ptr<UserInfo> user_info);
private:
    bool load_pending_;
    ConUserItem* add_friend_item_;
    QListWidgetItem * groupitem_;
};

#endif // CONTACTUSERLIST_H
