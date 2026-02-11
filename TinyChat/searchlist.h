#ifndef SEARCHLIST_H
#define SEARCHLIST_H
#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>
#include <QDialog>
#include <memory>
#include "userdata.h"
#include "loadingdlg.h"

/******************************************************************************
 * @file       searchlist.h
 * @brief      聊天侧栏搜索列表类
 *
 * @author     lueying
 * @date       2026/1/27
 * @history
 *****************************************************************************/

class SearchList : public QListWidget {
    Q_OBJECT
public:
    SearchList(QWidget* parent = nullptr);
    void closeFindDlg();
    void setSearchEdit(QWidget* edit);
protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        // 检查事件是否是鼠标悬浮进入或离开
        if (watched == this->viewport()) {
            if (event->type() == QEvent::Enter) {
                // 鼠标悬浮，显示滚动条
                this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            }
            else if (event->type() == QEvent::Leave) {
                // 鼠标离开，隐藏滚动条
                this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            }
        }

        // 检查事件是否是鼠标滚轮事件
        if (watched == this->viewport() && event->type() == QEvent::Wheel) {
            QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
            int numDegrees = wheelEvent->angleDelta().y() / 8;
            int numSteps = numDegrees / 15; // 计算滚动步数

            // 设置滚动幅度
            this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

            return true; // 停止事件传递
        }

        return QListWidget::eventFilter(watched, event);
    }
private:
    bool send_pending_; // 是否有发送消息等待发送
    std::shared_ptr<QDialog> find_dlg_;
    QWidget* search_edit_;
    LoadingDlg* loadingDialog_;

    // 根据pending状态展示加载框
    void waitPending(bool pending = true);
    // 添加列表条目
    void addTipItem();

private slots:
    void slot_item_clicked(QListWidgetItem* item);          // 搜索结果列表条目的点击槽函数
    void slot_user_search(std::shared_ptr<SearchInfo> si);  // 搜索完成槽函数
signals:
    void sig_jump_chat_item(std::shared_ptr<SearchInfo> si);// 跳转到聊天页面槽函数
};

#endif // SEARCHLIST_H

