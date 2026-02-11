#include "conuseritem.h"
#include "ui_conuseritem.h"

/******************************************************************************
 * @file       conuseritem.cpp
 * @brief      联系人列表构件类实现
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

ConUserItem::ConUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ConUserItem)
{
    ui->setupUi(this);
    setItemType(ListItemType::ContactUserItem);
    ui->red_point->raise();
    showRedPoint(false);
}

ConUserItem::~ConUserItem()
{
    delete ui;
}

QSize ConUserItem::sizeHint() const {
    return QSize(250, 70); // 返回自定义的尺寸
}

void ConUserItem::setInfo(std::shared_ptr<AuthInfo> auth_info) {
    info_ = std::make_shared<UserInfo>(auth_info);
    // 加载图片
    QPixmap pixmap(info_->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(info_->_name);
}

void ConUserItem::setInfo(int uid, QString name, QString icon) {
     info_ = std::make_shared<UserInfo>(uid,name, name, icon, 0);

     // 加载图片
     QPixmap pixmap(info_->_icon);

     // 设置图片自动缩放
     ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
     ui->icon_lb->setScaledContents(true);

     ui->user_name_lb->setText(info_->_name);
}

void ConUserItem::setInfo(std::shared_ptr<AuthRsp> auth_rsp){
    info_ = std::make_shared<UserInfo>(auth_rsp);

    // 加载图片
    QPixmap pixmap(info_->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(info_->_name);
}

void ConUserItem::showRedPoint(bool show) {
    if(show){
        ui->red_point->show();
    }else{
        ui->red_point->hide();
    }

}

std::shared_ptr<UserInfo> ConUserItem::getInfo() {
    return info_;
}
