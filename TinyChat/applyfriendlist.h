#ifndef APPLYFRIENDLIST_H
#define APPLYFRIENDLIST_H
#include <QListWidget>
#include <QEvent>

/******************************************************************************
 * @file       applyfriendlist.h
 * @brief      申请列表类
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

class ApplyFriendList: public QListWidget
{
     Q_OBJECT
public:
    ApplyFriendList(QWidget *parent = nullptr);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:

signals:
    void sig_show_search(bool);
};

#endif // APPLYFRIENDLIST_H
