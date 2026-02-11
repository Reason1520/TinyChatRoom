#ifndef STATEWIDGET_H
#define STATEWIDGET_H
#include <QWidget>
#include "global.h"
#include <QLabel>

/******************************************************************************
 * @file       statewidget.h
 * @brief      侧边栏按钮类，希望点击一个按钮，清空其他按钮的选中状态，有新的通知的时候出现红点的图标
 *
 * @author     lueying
 * @date       2026/1/27
 * @history
 *****************************************************************************/

class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);

    void setState(QString normal="", QString hover="", QString press="",
                  QString select="", QString select_hover="", QString select_press="");

    ClickLbState getCurState();
    void clearState();

    void setSelected(bool bselected);
    // 小红点
    void addRedPoint();
    void showRedPoint(bool show=true);

protected:
    void paintEvent(QPaintEvent* event);

    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

private:

    QString normal_;
    QString normal_hover_;
    QString normal_press_;

    QString selected_;
    QString selected_hover_;
    QString selected_press_;

    ClickLbState curstate_;
    QLabel * red_point_;

signals:
    void clicked(void);

signals:

public slots:
};

#endif // STATEWIDGET_H
