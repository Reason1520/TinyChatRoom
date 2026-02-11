#ifndef STATELABEL_H
#define STATELABEL_H
#include <QLabel>
#include "global.h"
#include <QMouseEvent>

/******************************************************************************
 * @file       chatpage.h
 * @brief      多种交互状态的可点击标签类
 *
 * @author     lueying
 * @date       2026/1/21
 * @history
 *****************************************************************************/

class StateLabel : public QLabel {
    Q_OBJECT
public:
    StateLabel(QWidget* parent = nullptr);
    virtual void mousePressEvent(QMouseEvent* ev) override;
    virtual void mouseReleaseEvent(QMouseEvent* ev) override;
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;
    void setState(QString normal = "", QString hover = "", QString press = "",
        QString select = "", QString select_hover = "", QString select_press = "");

    ClickLbState getCurState();
    void clearState();

    void setSelected(bool bselected);
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
    void clicked(void);
};

#endif // STATELABEL_H