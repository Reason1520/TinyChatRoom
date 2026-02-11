#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QPushButton>
#include <QTimer>

/******************************************************************************
 * @file       timerbtn.h
 * @brief      带计时器的按钮
 *
 * @author     lueying
 * @date       2026/1/1
 * @history
 *****************************************************************************/

class TimerBtn : public QPushButton {
public:
    TimerBtn(QWidget* parent = nullptr);
    ~TimerBtn();

    // 重写mouseReleaseEvent
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
private:
    QTimer* timer_;
    int counter_;
};

#endif // TIMERBTN_H