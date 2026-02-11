#include "listitembase.h"
#include <QStyleOption>
#include <QPainter>

/******************************************************************************
 * @file       listitembase.cpp
 * @brief      列表元素基类实现
 *
 * @author     lueying
 * @date       2026/1/18
 * @history
 *****************************************************************************/

ListItemBase::ListItemBase(QWidget *parent) : QWidget(parent) {

}

void ListItemBase::setItemType(ListItemType itemType) {
    itemType_ = itemType;
}

ListItemType ListItemBase::getItemType() {
    return itemType_;
}

void ListItemBase::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}