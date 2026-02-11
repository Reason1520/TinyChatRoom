#include "clickedlabel.h"
#include <QMouseEvent>

/******************************************************************************
 * @file       clickedlabel.h
 * @brief      可变换样式的label实现
 *
 * @author     lueying
 * @date       2026/1/2
 * @history
 *****************************************************************************/

ClickedLabel::ClickedLabel(QWidget* parent) :QLabel(parent), curstate_(ClickLbState::Normal)
{

}

// 处理鼠标点击事件
void ClickedLabel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (curstate_ == ClickLbState::Normal) {
            qDebug() << "clicked , change to selected hover: " << selected_hover_;
            curstate_ = ClickLbState::Selected;
            setProperty("state", selected_hover_);
            repolish(this);
            update();

        }
        else {
            qDebug() << "clicked , change to normal hover: " << normal_hover_;
            curstate_ = ClickLbState::Normal;
            setProperty("state", normal_hover_);
            repolish(this);
            update();
        }
        emit clicked(this->text(), curstate_);
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

// 处理鼠标点击释放事件
void ClickedLabel::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (curstate_ == ClickLbState::Normal) {
            // qDebug()<<"ReleaseEvent , change to normal hover: "<< _normal_hover;
            setProperty("state", normal_hover_);
            repolish(this);
            update();

        }
        else {
            //  qDebug()<<"ReleaseEvent , change to select hover: "<< _selected_hover;
            setProperty("state", selected_hover_);
            repolish(this);
            update();
        }
        emit clicked(this->text(), curstate_);
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

// 处理鼠标悬停进入事件
void ClickedLabel::enterEvent(QEnterEvent* event) {
    // 在这里处理鼠标悬停进入的逻辑
    if (curstate_ == ClickLbState::Normal) {
        qDebug() << "enter , change to normal hover: " << normal_hover_;
        setProperty("state", normal_hover_);
        repolish(this);
        update();

    }
    else {
        qDebug() << "enter , change to selected hover: " << selected_hover_;
        setProperty("state", selected_hover_);
        repolish(this);
        update();
    }

    QLabel::enterEvent(event);
}

// 处理鼠标悬停离开事件
void ClickedLabel::leaveEvent(QEvent* event) {
    // 在这里处理鼠标悬停离开的逻辑
    if (curstate_ == ClickLbState::Normal) {
        qDebug() << "leave , change to normal : " << normal_;
        setProperty("state", normal_);
        repolish(this);
        update();

    }
    else {
        qDebug() << "leave , change to normal hover: " << selected_;
        setProperty("state", selected_);
        repolish(this);
        update();
    }
    QLabel::leaveEvent(event);
}

void ClickedLabel::setState(QString normal, QString hover, QString press,
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

ClickLbState ClickedLabel::getCurState() {
    return curstate_;
}

bool ClickedLabel::setCurState(ClickLbState state) {
    curstate_ = state;
    if (curstate_ == ClickLbState::Normal) {
        setProperty("state", normal_);
        repolish(this);
    }
    else if (curstate_ == ClickLbState::Selected) {
        setProperty("state", selected_);
        repolish(this);
    }

    return true;
}

void ClickedLabel::resetNormalState() {
    curstate_ = ClickLbState::Normal;
    setProperty("state", normal_);
    repolish(this);
}
