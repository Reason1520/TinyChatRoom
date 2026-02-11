#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QDialog>
#include "ui_mainwindow.h"
#include "logindialog.h"
#include "signupdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"

/******************************************************************************
 * @file       mainwindow.h
 * @brief      主窗口类
 *
 * @author     lueying
 * @date       2025/12/7
 * @history
 *****************************************************************************/

namespace Ui {
    class MainWindow;
}

enum UIStatus {
    LOGIN_UI,
    SIGNUP_UI,
    RESET_UI,
    CHAT_UI
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void login_to_sign_up();    // 从登录界面切换到注册界面的槽函数
    void sign_up_to_login();    // 从注册界面切换到登录界面的槽函数
    void login_to_reset();      // 从登录界面切换到重置密码界面的槽函数
    void reset_to_login();      // 从重置密码界面切换到登录界面的槽函数
    void login_to_chat();       // 登录成功后跳转到聊天界面槽函数
    void slotOffline();         // 下线槽函数
    void slotExcepConOffline(); // 异常断开连接槽函数

private:
    Ui::MainWindowClass* ui_;
    LoginDialog* login_dialog_;     // 登录对话框
    SignUpDialog* signup_dialog_;   // 注册对话框
    ResetDialog* reset_dialog_;     // 重置密码对话框
    ChatDialog* chat_dialog_;       // 聊天对话框
    UIStatus ui_status_;            // ui界面状态

    void offlineLogin();        // 下线后退出到登录界面
};

#endif // MAINWINDOW_H

