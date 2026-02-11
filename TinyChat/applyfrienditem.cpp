#include "applyfrienditem.h"
#include "ui_applyfrienditem.h"

/******************************************************************************
 * @file       applyfrienditem.h
 * @brief      添加好友页面构件类
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

ApplyFriendItem::ApplyFriendItem(QWidget *parent) :
    ListItemBase(parent), added_(false),
    ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    setItemType(ListItemType::ApplyFriendItem);
    ui->addBtn->setState("normal","hover", "press");
    ui->addBtn->hide();
    connect(ui->addBtn, &ClickedBtn::clicked,  [this](){
        emit this->sig_auth_friend(apply_info_);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::setInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    apply_info_ = apply_info;
    // 加载图片
    QPixmap pixmap(apply_info_->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(apply_info_->_name);
    ui->user_chat_lb->setText(apply_info_->_desc);
}

void ApplyFriendItem::showAddBtn(bool bshow)
{
    if (bshow) {
		ui->addBtn->show();
		ui->already_add_lb->hide();
        added_ = false;
    }
    else {
		ui->addBtn->hide();
		ui->already_add_lb->show();
        added_ = true;
    }
}

int ApplyFriendItem::getUid() {
    return apply_info_->_uid;
}


