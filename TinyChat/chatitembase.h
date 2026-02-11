#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include "global.h"

/******************************************************************************
 * @file       chatitembase.h
 * @brief      聊天消息类
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

class BubbleFrame;

class ChatItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);
    void setUserName(const QString &name);
    void setUserIcon(const QPixmap &icon);
    void setWidget(QWidget *w);
    void setStatus(int status);
    QLabel* getIconLabel();
    QWidget* getBubble();

private:
    ChatRole role_;
    QLabel *pNameLabel_;
    QLabel *pIconLabel_;
    // 气泡消息
    QWidget *pBubble_;
    // 状态
    QLabel* pStatusLabel_;
};

#endif // CHATITEMBASE_H
