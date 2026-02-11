#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"

/******************************************************************************
 * @file       conuseritem.h
 * @brief      联系人列表构件类
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

namespace Ui {
class ConUserItem;
}

class ConUserItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();
    QSize sizeHint() const override;
    void setInfo(std::shared_ptr<AuthInfo> auth_info);
    void setInfo(std::shared_ptr<AuthRsp> auth_rsp);
    void setInfo(int uid, QString name, QString icon);
    void showRedPoint(bool show = false);
    std::shared_ptr<UserInfo> getInfo();
private:
    Ui::ConUserItem *ui;
    std::shared_ptr<UserInfo> info_;
};

#endif // CONUSERITEM_H
