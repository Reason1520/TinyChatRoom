#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

/******************************************************************************
 * @file       chatview.h
 * @brief      聊天信息布局构件类
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

class ChatView : public QWidget
{
    Q_OBJECT
public:
    ChatView(QWidget* parent = Q_NULLPTR);
    // 添加消息
    void appendChatItem(QWidget* item);                     //尾插
    void prependChatItem(QWidget* item);                    //头插
    void insertChatItem(QWidget* before, QWidget* item);    //中间插
    // 清除消息
    void removeAllItem();
protected:
    // 重写时间过滤器
    bool eventFilter(QObject* o, QEvent* e) override;
    void paintEvent(QPaintEvent* event) override;
private slots:
    // 自动将滚动条滚动到底部的槽函数
    void onVScrollBarMoved(int min, int max);
private:
    void initStyleSheet();
private:
    //QWidget *pCenterWidget_;
    // 垂直布局
    QVBoxLayout*pVl_;
    // 滚动区域
    QScrollArea* pScrollArea_;
    bool isAppended;
};

#endif // CHATVIEW_H