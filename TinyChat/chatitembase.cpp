#include "ChatItemBase.h"
#include <QFont>
#include <QVBoxLayout>
#include "bubbleframe.h"

/******************************************************************************
 * @file       chatitembase.cpp
 * @brief      聊天消息类实现
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget(parent)
    , role_(role)
{
    pNameLabel_    = new QLabel();
    pNameLabel_->setObjectName("chat_user_name");
    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    pNameLabel_->setFont(font);
    pNameLabel_->setFixedHeight(20);
    pIconLabel_    = new QLabel();
    pIconLabel_->setScaledContents(true);
    pIconLabel_->setFixedSize(42, 42);
    pBubble_       = new QWidget();
    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->setVerticalSpacing(3);
    pGLayout->setHorizontalSpacing(3);
    pGLayout->setContentsMargins(3, 3, 3, 3);
    QSpacerItem*pSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    //添加状态图标控件
    pStatusLabel_ = new QLabel();
    pStatusLabel_->setFixedSize(16, 16);
    pStatusLabel_->setScaledContents(true);

    if (role_ == ChatRole::Self)
    {
        pNameLabel_->setContentsMargins(0, 0, 8, 0);
        pNameLabel_->setAlignment(Qt::AlignRight);
        //名字标签
        pGLayout->addWidget(pNameLabel_, 0, 2, 1, 1);
        //icon 头像
        pGLayout->addWidget(pIconLabel_, 0, 3, 2, 1, Qt::AlignTop);
        //第 0 列：依然是 pSpacer，占用第 1 行，第 0 列
        pGLayout->addItem(pSpacer, 1, 0, 1, 1);
        //气泡控件
        pGLayout->addWidget(pBubble_, 1, 2, 1, 1);
        //状态图标
        pGLayout->addWidget(pStatusLabel_, 1, 1, 1, 1, Qt::AlignCenter);
        pGLayout->setColumnStretch(0, 2);
        pGLayout->setColumnStretch(1, 0);  // status 图标 (固定大小)
        pGLayout->setColumnStretch(2, 3);  // 名字 + 气泡 (主要拉伸区域)
        pGLayout->setColumnStretch(3, 0);  // 头像 (固定大小)
    }
    else {
        pNameLabel_->setContentsMargins(8, 0, 0, 0);
        pNameLabel_->setAlignment(Qt::AlignLeft);
        pGLayout->addWidget(pIconLabel_, 0, 0, 2, 1, Qt::AlignTop);
        pGLayout->addWidget(pNameLabel_, 0, 1, 1, 1);
        pGLayout->addWidget(pBubble_, 1, 1, 1, 1);
        pGLayout->addItem(pSpacer, 2, 2, 1, 1);
        pGLayout->setColumnStretch(1, 3);
        pGLayout->setColumnStretch(2, 2);
    }
    this->setLayout(pGLayout);
}

void ChatItemBase::setUserName(const QString &name) {
    pNameLabel_->setText(name);
}

void ChatItemBase::setUserIcon(const QPixmap &icon) {
    pIconLabel_->setPixmap(icon);
}

void ChatItemBase::setWidget(QWidget *w) {
   QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
   pGLayout->replaceWidget(pBubble_, w);
   delete pBubble_;
   pBubble_ = w;
}

void ChatItemBase::setStatus(int status) {
    if (status == MsgStatus::UN_READ) {
        pStatusLabel_->setPixmap(QPixmap(":/res/unread.png"));
        return;
    }

    if (status == MsgStatus::SEND_FAILED) {
        pStatusLabel_->setPixmap(QPixmap(":/res/send_fail.png"));
        return;
    }

    if (status == MsgStatus::READED) {
        pStatusLabel_->setPixmap(QPixmap(":/res/readed.png"));
        return;
    }
}

QLabel* ChatItemBase::getIconLabel() {
    return pIconLabel_;
}

QWidget* ChatItemBase::getBubble() {
    return pBubble_;
}
