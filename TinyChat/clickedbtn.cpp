#include "clickedbtn.h"
#include <QVariant>
#include "global.h"

/******************************************************************************
 * @file       clickedbtn.h
 * @brief      拥有点击效果的按钮类实现
 *
 * @author     lueying
 * @date       2026/1/7
 * @history
 *****************************************************************************/

ClickedBtn::ClickedBtn(QWidget* parent) :QPushButton(parent) {
    setCursor(Qt::PointingHandCursor); // 设置光标为小手
}

ClickedBtn::~ClickedBtn() {

}

void ClickedBtn::setState(QString normal, QString hover, QString press) {
    hover_ = hover;
    normal_ = normal;
    press_ = press;
    setProperty("state", normal);
    repolish(this);
    update();
}

void ClickedBtn::enterEvent(QEnterEvent* event) {
    setProperty("state", hover_);
    repolish(this);
    update();
    QPushButton::enterEvent(event);
}

void ClickedBtn::leaveEvent(QEvent* event) {
    setProperty("state", normal_);
    repolish(this);
    update();
    QPushButton::leaveEvent(event);
}

void ClickedBtn::mousePressEvent(QMouseEvent* event) {
    setProperty("state", press_);
    repolish(this);
    update();
    QPushButton::mousePressEvent(event);
}

void ClickedBtn::mouseReleaseEvent(QMouseEvent* event) {
    setProperty("state", hover_);
    repolish(this);
    update();
    QPushButton::mouseReleaseEvent(event);
}