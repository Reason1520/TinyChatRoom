#include "BubbleFrame.h"
#include <QPainter>
#include <QDebug>

/******************************************************************************
 * @file       bubbleframe.h
 * @brief      聊天消息气泡基类
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

const int WIDTH_SANJIAO  = 8;  //三角宽
BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent)
    :QFrame(parent)
    ,role_(role)
    ,margin_(3)
{
    pHLayout_ = new QHBoxLayout();
    if(role_ == ChatRole::Self)
        pHLayout_->setContentsMargins(margin_, margin_, WIDTH_SANJIAO + margin_, margin_);
    else
        pHLayout_->setContentsMargins(WIDTH_SANJIAO + margin_, margin_, margin_, margin_);

    this->setLayout(pHLayout_);
}

void BubbleFrame::setMargin(int margin) {
    Q_UNUSED(margin);
    //m_margin = margin;
}

void BubbleFrame::setWidget(QWidget *w) {
    if(pHLayout_->count() > 0)
        return ;
    else{
        pHLayout_->addWidget(w);
    }
}

void BubbleFrame::paintEvent(QPaintEvent *e) {
    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    if(role_ == ChatRole::Other)
    {
        //画气泡
        QColor bk_color(Qt::white);
        painter.setBrush(QBrush(bk_color));
        QRect bk_rect = QRect(WIDTH_SANJIAO, 0, this->width()-WIDTH_SANJIAO, this->height());
        painter.drawRoundedRect(bk_rect,5,5);
        //画小三角
        QPointF points[3] = {
            QPointF(bk_rect.x(), 12),
            QPointF(bk_rect.x(), 10+WIDTH_SANJIAO +2),
            QPointF(bk_rect.x()-WIDTH_SANJIAO, 10+WIDTH_SANJIAO-WIDTH_SANJIAO/2),
        };
        painter.drawPolygon(points, 3);
    }
    else
    {
        QColor bk_color(158,234,106);
        painter.setBrush(QBrush(bk_color));
        //画气泡
        QRect bk_rect = QRect(0, 0, this->width()-WIDTH_SANJIAO, this->height());
        painter.drawRoundedRect(bk_rect,5,5);
        //画三角
        QPointF points[3] = {
            QPointF(bk_rect.x()+bk_rect.width(), 12),
            QPointF(bk_rect.x()+bk_rect.width(), 12+WIDTH_SANJIAO +2),
            QPointF(bk_rect.x()+bk_rect.width()+WIDTH_SANJIAO, 10+WIDTH_SANJIAO-WIDTH_SANJIAO/2),
        };
        painter.drawPolygon(points, 3);

    }

    return QFrame::paintEvent(e);
}
