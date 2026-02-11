#include "clickedoncelabel.h"

/******************************************************************************
 * @file       clickedoncelabel.h
 * @brief      支持点击的label类
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

ClickedOnceLabel::ClickedOnceLabel(QWidget *parent):QLabel(parent)
{
    // 鼠标悬停变成小手
    setCursor(Qt::PointingHandCursor);
}


void ClickedOnceLabel::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(this->text());
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

