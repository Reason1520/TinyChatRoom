#ifndef LOADINGDLG_H
#define LOADINGDLG_H

#include <QDialog>

/******************************************************************************
 * @file       loadingdlg.h
 * @brief      加载动画部件类
 *
 * @author     lueying
 * @date       2026/1/20
 * @history
 *****************************************************************************/

namespace Ui {
class LoadingDlg;
}

class LoadingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingDlg(QWidget* parent = nullptr, QString tip = "Loading...");
    ~LoadingDlg();

private:
    Ui::LoadingDlg *ui;
};

#endif // LOADINGDLG_H
