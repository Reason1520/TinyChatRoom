#ifndef APPLYFRIENDITEM_H
#define APPLYFRIENDITEM_H

#include <QWidget>
#include <listitembase.h>
#include "userdata.h"
#include <memory>

/******************************************************************************
 * @file       applyfrienditem.h
 * @brief      添加好友页面构件类
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

namespace Ui {
class ApplyFriendItem;
}

class ApplyFriendItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ApplyFriendItem(QWidget *parent = nullptr);
    ~ApplyFriendItem();
    void setInfo(std::shared_ptr<ApplyInfo> apply_info);
    void showAddBtn(bool bshow);
    QSize sizeHint() const override {
        return QSize(250, 80); // 返回自定义的尺寸
    }
    int getUid();
private:
    Ui::ApplyFriendItem *ui;
    std::shared_ptr<ApplyInfo> apply_info_;
    bool added_;
signals:
    void sig_auth_friend(std::shared_ptr<ApplyInfo> apply_info);
};

#endif // APPLYFRIENDITEM_H
