#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H
#include <QLabel>
#include "global.h"

/******************************************************************************
 * @file       clickedlabel.h
 * @brief      可变换样式的label
 *
 * @author     lueying
 * @date       2026/1/2
 * @history
 *****************************************************************************/

class ClickedLabel :public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(QWidget* parent);
    // 事件重写
    virtual void mousePressEvent(QMouseEvent* ev) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

    // 设置样式
    void setState(QString normal = "", QString hover = "", QString press = "",
        QString select = "", QString select_hover = "", QString select_press = "");
    // 获取当前状态
    ClickLbState getCurState();
    // 设置当前状态
    bool setCurState(ClickLbState state);
    // 重置到初始状态
    void resetNormalState();
protected:

private:
    QString normal_;
    QString normal_hover_;
    QString normal_press_;

    QString selected_;
    QString selected_hover_;
    QString selected_press_;

    ClickLbState curstate_;
signals:
    void clicked(QString, ClickLbState);

};

#endif // CLICKEDLABEL_H

