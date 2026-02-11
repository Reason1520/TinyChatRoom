#ifndef BUBBLE_H
#define BUBBLE_H

#include <QFrame>
#include "global.h"
#include <QHBoxLayout>

/******************************************************************************
 * @file       bubbleframe.h
 * @brief      聊天消息气泡基类
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

class BubbleFrame : public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role, QWidget *parent = nullptr);
    void setMargin(int margin);
    //inline int margin(){return margin;}
    void setWidget(QWidget *w);
protected:
    void paintEvent(QPaintEvent *e);
private:
    QHBoxLayout *pHLayout_;
    ChatRole role_;
     int      margin_;
};

#endif // BUBBLE_H
