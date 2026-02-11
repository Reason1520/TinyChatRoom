#include "chatview.h"
#include <QScrollBar>
#include <QEvent>
#include <QStyleOption>
#include <QPainter>
#include <QTimer>

/******************************************************************************
 * @file       chatview.cpp
 * @brief      聊天信息布局构件类实现
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

ChatView::ChatView(QWidget* parent) : QWidget(parent)
, isAppended(false) {
    QVBoxLayout* pMainLayout = new QVBoxLayout();
    this->setLayout(pMainLayout);
    pMainLayout->setContentsMargins(0, 0, 0, 0);

    pScrollArea_ = new QScrollArea();
    pScrollArea_->setObjectName("chat_area");
    pMainLayout->addWidget(pScrollArea_);

    QWidget* w = new QWidget(this);
    w->setObjectName("chat_bg");
    w->setAutoFillBackground(true);

    QVBoxLayout* pVLayout_1 = new QVBoxLayout();
    // 添加弹簧
    pVLayout_1->addWidget(new QWidget(), 100000);
    w->setLayout(pVLayout_1);
    pScrollArea_->setWidget(w);

    // 隐藏原生边框滚动条
    pScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar* pVScrollBar = pScrollArea_->verticalScrollBar();
    // 将滚动条范围变化的信号连接到滚动条滚动到底部的槽函数
    connect(pVScrollBar, &QScrollBar::rangeChanged, this, &ChatView::onVScrollBarMoved);

    //把垂直ScrollBar放到上边 而不是原来的并排
    QHBoxLayout* pHLayout_2 = new QHBoxLayout();
    pHLayout_2->addWidget(pVScrollBar, 0, Qt::AlignRight);
    pHLayout_2->setContentsMargins(0, 0, 0, 0);
    pScrollArea_->setLayout(pHLayout_2);
    pVScrollBar->setHidden(true);

    pScrollArea_->setWidgetResizable(true);
    pScrollArea_->installEventFilter(this);
    initStyleSheet();
}

// 尾插聊天信息
void ChatView::appendChatItem(QWidget* item) {
    QVBoxLayout* vl = qobject_cast<QVBoxLayout*>(pScrollArea_->widget()->layout());
    vl->insertWidget(vl->count() - 1, item);
    isAppended = true;
}

void ChatView::prependChatItem(QWidget* item)
{

}

void ChatView::insertChatItem(QWidget* before, QWidget* item)
{

}

// 清除所有构件（聊天消息）
void ChatView::removeAllItem() {
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(pScrollArea_->widget()->layout());

    int count = layout->count();

    for (int i = 0; i < count - 1; ++i) {
        QLayoutItem* item = layout->takeAt(0); // 始终从第一个控件开始删除
        if (item) {
            if (QWidget* widget = item->widget()) {
                delete widget;
            }
            delete item;
        }
    }

}

// 重写事件过滤器
bool ChatView::eventFilter(QObject* o, QEvent* e) {
    /*if(e->type() == QEvent::Resize && o == )
    {

    }
    else */if (e->type() == QEvent::Enter && o == pScrollArea_)
    {
        // 只有在用户可能需要滚动且内容确实可滚动时，才展示滚动条
        pScrollArea_->verticalScrollBar()->setHidden(pScrollArea_->verticalScrollBar()->maximum() == 0);
    }
    else if (e->type() == QEvent::Leave && o == pScrollArea_)
    {
        pScrollArea_->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(o, e);
}

// 重写绘制事件
void ChatView::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// 自动将滚动条滚动到底部的槽函数
void ChatView::onVScrollBarMoved(int min, int max) {
    if (isAppended) // 只有当真的添加的消息时才调用，添加item可能调用多次
    {
        QScrollBar* pVScrollBar = pScrollArea_->verticalScrollBar();
        pVScrollBar->setSliderPosition(pVScrollBar->maximum());
        //500毫秒内可能调用多次，可能还未计算完成，先锁定在底部
        QTimer::singleShot(500, [this]()
            {
                isAppended = false;
            });
    }
}

void ChatView::initStyleSheet()
{
    //    QScrollBar *scrollBar = m_pScrollArea->verticalScrollBar();
    //    scrollBar->setStyleSheet("QScrollBar{background:transparent;}"
    //                             "QScrollBar:vertical{background:transparent;width:8px;}"
    //                             "QScrollBar::handle:vertical{background:red; border-radius:4px;min-height:20px;}"
    //                             "QScrollBar::add-line:vertical{height:0px}"
    //                             "QScrollBar::sub-line:vertical{height:0px}"
    //                             "QScrollBar::add-page:vertical {background:transparent;}"
    //                             "QScrollBar::sub-page:vertical {background:transparent;}");
}