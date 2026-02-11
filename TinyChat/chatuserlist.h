#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H

#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>

/******************************************************************************
 * @file       chatuserlish.h
 * @brief      聊天信息列表类
 *
 * @author     lueying
 * @date       2026/1/8
 * @history
 *****************************************************************************/

class ChatUserList : public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget* parent = nullptr);
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void sig_loading_chat_user();
};

#endif // CHATUSERLIST_H

