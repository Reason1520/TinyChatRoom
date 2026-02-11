#ifndef FINDFAILDLG_H
#define FINDFAILDLG_H

#include <QDialog>

/******************************************************************************
 * @file       findfaildlg.h
 * @brief      聊天侧栏搜索失败对话框类
 *
 * @author     lueying
 * @date       2026/1/27
 * @history
 *****************************************************************************/

namespace Ui {
class FindFailDlg;
}

class FindFailDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FindFailDlg(QWidget *parent = nullptr);
    ~FindFailDlg();

private slots:


    void on_fail_sure_btn_clicked();

private:
    Ui::FindFailDlg *ui;
};

#endif // FINDFAILDLG_H
