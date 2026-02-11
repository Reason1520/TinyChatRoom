#include "statelabel.h"

/******************************************************************************
 * @file       chatpage.h
 * @brief      多种交互状态的可点击标签类
 *
 * @author     lueying
 * @date       2026/1/21
 * @history
 *****************************************************************************/

StateLabel::StateLabel(QWidget* parent) :QLabel(parent), curstate_(ClickLbState::Normal) {
    setCursor(Qt::PointingHandCursor);
}

// 处理鼠标点击事件
void StateLabel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (curstate_ == ClickLbState::Selected) {
            qDebug() << "PressEvent , already to selected press: " << selected_press_;
            emit clicked();
            // 调用基类的mousePressEvent以保证正常的事件处理
            QLabel::mousePressEvent(event);
            return;
        }

        if (curstate_ == ClickLbState::Normal) {
            qDebug() << "PressEvent , change to selected press: " << selected_press_;
            curstate_ = ClickLbState::Selected;
            setProperty("state", selected_press_);
            repolish(this);
            update();
        }

        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

void StateLabel::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (curstate_ == ClickLbState::Normal) {
            //qDebug()<<"ReleaseEvent , change to normal hover: "<< _normal_hover;
            setProperty("state", normal_hover_);
            repolish(this);
            update();

        }
        else {
            //qDebug()<<"ReleaseEvent , change to select hover: "<< _selected_hover;
            setProperty("state", selected_hover_);
            repolish(this);
            update();
        }
        emit clicked();
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

// 处理鼠标悬停进入事件
void StateLabel::enterEvent(QEnterEvent* event) {
    // 在这里处理鼠标悬停进入的逻辑
    if (curstate_ == ClickLbState::Normal) {
        //qDebug()<<"enter , change to normal hover: "<< _normal_hover;
        setProperty("state", normal_hover_);
        repolish(this);
        update();

    }
    else {
        //qDebug()<<"enter , change to selected hover: "<< _selected_hover;
        setProperty("state", selected_hover_);
        repolish(this);
        update();
    }

    QLabel::enterEvent(event);
}

// 处理鼠标悬停离开事件
void StateLabel::leaveEvent(QEvent* event) {
    // 在这里处理鼠标悬停离开的逻辑
    if (curstate_ == ClickLbState::Normal) {
        // qDebug()<<"leave , change to normal : "<< _normal;
        setProperty("state", normal_);
        repolish(this);
        update();

    }
    else {
        // qDebug()<<"leave , change to select normal : "<< _selected;
        setProperty("state", selected_);
        repolish(this);
        update();
    }
    QLabel::leaveEvent(event);
}

void StateLabel::setState(QString normal, QString hover, QString press,
    QString select, QString select_hover, QString select_press) {
    normal_ = normal;
    normal_hover_ = hover;
    normal_press_ = press;

    selected_ = select;
    selected_hover_ = select_hover;
    selected_press_ = select_press;

    setProperty("state", normal);
    repolish(this);
}

ClickLbState StateLabel::getCurState() {
    return curstate_;
}

void StateLabel::clearState() {
    curstate_ = ClickLbState::Normal;
    setProperty("state", normal_);
    repolish(this);
    update();
}

void StateLabel::setSelected(bool bselected) {
    if (bselected) {
        curstate_ = ClickLbState::Selected;
        setProperty("state", selected_);
        repolish(this);
        update();
        return;
    }

    curstate_ = ClickLbState::Normal;
    setProperty("state", normal_);
    repolish(this);
    update();
    return;

}



