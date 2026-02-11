#include "statewidget.h"
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>

/******************************************************************************
 * @file       statewidget.cpp
 * @brief      侧边栏按钮类实现
 *
 * @author     lueying
 * @date       2026/1/27
 * @history
 *****************************************************************************/

StateWidget::StateWidget(QWidget *parent) : QWidget(parent),curstate_(ClickLbState::Normal) {
    setCursor(Qt::PointingHandCursor);
    //添加红点
    addRedPoint();
}

void StateWidget::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    return;

}

// 处理鼠标点击事件
void StateWidget::mousePressEvent(QMouseEvent* event)  {
    if (event->button() == Qt::LeftButton) {
        if(curstate_ == ClickLbState::Selected){
            qDebug()<<"PressEvent , already to selected press: "<< selected_press_;
            //emit clicked();
            // 调用基类的mousePressEvent以保证正常的事件处理
            QWidget::mousePressEvent(event);
            return;
        }

        if(curstate_ == ClickLbState::Normal){
            qDebug()<<"PressEvent , change to selected press: "<< selected_press_;
            curstate_ = ClickLbState::Selected;
            setProperty("state",selected_press_);
            repolish(this);
            update();
        }

        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(curstate_ == ClickLbState::Normal){
            //qDebug()<<"ReleaseEvent , change to normal hover: "<< _normal_hover;
            setProperty("state",normal_hover_);
            repolish(this);
            update();

        }else{
            //qDebug()<<"ReleaseEvent , change to select hover: "<< _selected_hover;
            setProperty("state",selected_hover_);
            repolish(this);
            update();
        }
        emit clicked();
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}

// 处理鼠标悬停进入事件
void StateWidget::enterEvent(QEnterEvent* event) {
    // 在这里处理鼠标悬停进入的逻辑
    if(curstate_ == ClickLbState::Normal){
         //qDebug()<<"enter , change to normal hover: "<< _normal_hover;
        setProperty("state",normal_hover_);
        repolish(this);
        update();

    }else{
         //qDebug()<<"enter , change to selected hover: "<< _selected_hover;
        setProperty("state",selected_hover_);
        repolish(this);
        update();
    }

    QWidget::enterEvent(event);
}

// 处理鼠标悬停离开事件
void StateWidget::leaveEvent(QEvent* event){
    // 在这里处理鼠标悬停离开的逻辑
    if(curstate_ == ClickLbState::Normal){
        // qDebug()<<"leave , change to normal : "<< _normal;
        setProperty("state",normal_);
        repolish(this);
        update();

    }else{
        // qDebug()<<"leave , change to select normal : "<< _selected;
        setProperty("state",selected_);
        repolish(this);
        update();
    }
    QWidget::leaveEvent(event);
}

void StateWidget::setState(QString normal, QString hover, QString press,
                            QString select, QString select_hover, QString select_press) {
    normal_ = normal;
    normal_hover_ = hover;
    normal_press_ = press;

    selected_ = select;
    selected_hover_ = select_hover;
    selected_press_ = select_press;

    setProperty("state",normal_);
    repolish(this);
}

ClickLbState StateWidget::getCurState() {
    return curstate_;
}

void StateWidget::clearState() {
    curstate_ = ClickLbState::Normal;
    setProperty("state",normal_);
    repolish(this);
    update();
}

void StateWidget::setSelected(bool bselected) {
    if(bselected){
        curstate_ = ClickLbState::Selected;
        setProperty("state",selected_);
        repolish(this);
        update();
        return;
    }

    curstate_ = ClickLbState::Normal;
    setProperty("state",normal_);
    repolish(this);
    update();
    return;

}

void StateWidget::addRedPoint() {
    //添加红点示意图
    red_point_ = new QLabel();
    red_point_->setObjectName("red_point");

    // 加载红点图片资源
    QPixmap pixmap(":/res/red_point.png");
    red_point_->setPixmap(pixmap);
    red_point_->setScaledContents(true);

    QVBoxLayout* layout2 = new QVBoxLayout;
    red_point_->setAlignment(Qt::AlignCenter);
    layout2->addWidget(red_point_);
    layout2->setContentsMargins(0,0,0,0);
    this->setLayout(layout2);
    red_point_->setVisible(false);
}

void StateWidget::showRedPoint(bool show) {
    red_point_->setVisible(show);
}





