#ifndef CLICKEDONCELABEL_H
#define CLICKEDONCELABEL_H
#include <QLabel>
#include <QMouseEvent>

/******************************************************************************
 * @file       clickedoncelabel.h
 * @brief      支持点击的label类
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

class ClickedOnceLabel:public QLabel
{
    Q_OBJECT
public:
    ClickedOnceLabel(QWidget *parent=nullptr);
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;

signals:
    void clicked(QString );
};

#endif // CLICKEDONCELABEL_H
