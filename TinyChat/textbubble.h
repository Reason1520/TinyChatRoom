#ifndef TEXTBUBBLE_H
#define TEXTBUBBLE_H

#include <QTextEdit>
#include "bubbleframe.h"
#include <QHBoxLayout>

/******************************************************************************
 * @file       textbubble.h
 * @brief      聊天消息文字气泡类
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

class TextBubble : public BubbleFrame
{
    Q_OBJECT
public:
    TextBubble(ChatRole role, const QString &text, QWidget *parent = nullptr);
protected:
    bool eventFilter(QObject *o, QEvent *e);
private:
    // 调整文本高度
    void adjustTextHeight();
    // 设置文本最大宽度
    void setPlainText(const QString &text);
    // 设置样式表
    void initStyleSheet();
private:
    QTextEdit *pTextEdit_;
};

#endif // TEXTBUBBLE_H
