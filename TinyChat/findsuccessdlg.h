#ifndef FINDSUCCESSDLG_H
#define FINDSUCCESSDLG_H

#include <QDialog>
#include <memory>
#include "userdata.h"

/******************************************************************************
 * @file       findsuccessdlg.h
 * @brief      聊天侧栏搜索成功对话框类
 *
 * @author     lueying
 * @date       2026/1/27
 * @history
 *****************************************************************************/

namespace Ui {
class FindSuccessDlg;
}

class FindSuccessDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FindSuccessDlg(QWidget *parent = nullptr);
    ~FindSuccessDlg();
    void setSearchInfo(std::shared_ptr<SearchInfo> si);
private slots:
    void on_add_friend_btn_clicked();

private:
    Ui::FindSuccessDlg *ui;
    QWidget * parent_;
    std::shared_ptr<SearchInfo> si_;
};

#endif // FINDSUCCESSDLG_H
