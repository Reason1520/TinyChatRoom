#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include <QDialog>
#include "clickedlabel.h"
#include "friendlabel.h"
#include "userdata.h"

/******************************************************************************
 * @file       applyfriend.h
 * @brief      申请好友页面类
 *
 * @author     lueying
 * @date       2026/1/28
 * @history
 *****************************************************************************/

namespace Ui {
class ApplyFriend;
}

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();
    // 模拟创建标签
    void initTipLbs();
    // 增加标签到展示区
    void addTipLbs(ClickedLabel*, QPoint cur_point, QPoint &next_point, int text_width, int text_height);
    // 重写事件过滤器展示滑动条
    bool eventFilter(QObject *obj, QEvent *event);
    // 设置搜索信息
    void setSearchInfo(std::shared_ptr<SearchInfo> si);
private:
    Ui::ApplyFriend *ui;
    //已经创建好的标签
    QMap<QString, ClickedLabel*> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;
    //用来在输入框显示添加新好友的标签
    QMap<QString, FriendLabel*> _friend_labels;
    std::vector<QString> _friend_label_keys;
    std::vector<QString> _tip_data;
    QPoint _tip_cur_point;
    std::shared_ptr<SearchInfo> _si;

    // 重排好友标签编辑栏的标签
    void resetLabels();
    // 添加好友标签编辑栏的标签
    void addLabel(QString name);
public slots:
    //显示更多label标签
    void ShowMoreLabel();
    //输入label按下回车触发将标签加入展示栏
    void SlotLabelEnter();
    //点击关闭，移除展示栏好友便签
    void SlotRemoveFriendLabel(QString);
    //通过点击tip实现增加和减少好友便签
    void SlotChangeFriendLabelByTip(QString, ClickLbState);
    //输入框文本变化显示不同提示
    void SlotLabelTextChange(const QString& text);
    //输入框输入完成
    void SlotLabelEditFinished();
   //输入标签显示提示框，点击提示框内容后添加好友便签
    void SlotAddFirendLabelByClickTip(QString text);
    // 确认添加好友槽函数
    void SlotApplySure();
    // 取消添加好友槽函数
    void SlotApplyCancel();
};

#endif // APPLYFRIEND_H
