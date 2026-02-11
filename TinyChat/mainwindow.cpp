#include "mainwindow.h"
#include "tcpmgr.h"
#include <QMessageBox>

/******************************************************************************
 * @file       mainwindow.cpp
 * @brief      主窗口类实现文件
 *
 * @author     lueying
 * @date       2025/12/7
 * @history
 *****************************************************************************/

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindowClass) {
    ui_->setupUi(this);
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    login_dialog_ = new LoginDialog(this);
    login_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(login_dialog_);

    //连接登录界面注册信号
    connect(login_dialog_, &LoginDialog::switch_sign_up, this, &MainWindow::login_to_sign_up);
    //连接登录界面忘记密码信号
    connect(login_dialog_, &LoginDialog::switch_reset, this, &MainWindow::login_to_reset);
    //连接创建聊天界面信号
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_swich_chatdlg, this, &MainWindow::login_to_chat);
    //链接服务器踢人消息
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_notify_offline, this, &MainWindow::slotOffline);
    //连接服务器断开心跳超时或异常连接信息
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_connection_closed, this, &MainWindow::slotExcepConOffline);
}

MainWindow::~MainWindow() {
    if (ui_) {
        delete ui_;
    }
}

// 从登录界面切换到注册界面的槽函数
void MainWindow::login_to_sign_up() {
    signup_dialog_ = new SignUpDialog(this);
    signup_dialog_->hide();

    signup_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    //连接注册界面返回登录信号
    connect(signup_dialog_, &SignUpDialog::sign_up_switch_login, this, &MainWindow::sign_up_to_login);
    setCentralWidget(signup_dialog_);
    login_dialog_->hide();
    signup_dialog_->show();
    ui_status_ = SIGNUP_UI;
}

//从注册界面返回登录界面
void MainWindow::sign_up_to_login() {
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    login_dialog_ = new LoginDialog(this);
    login_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(login_dialog_);

    signup_dialog_->hide();
    login_dialog_->show();
    //连接登录界面注册信号
    connect(login_dialog_, &LoginDialog::switch_sign_up, this, &MainWindow::login_to_sign_up);
    //连接登录界面忘记密码信号
    connect(login_dialog_, &LoginDialog::switch_reset, this, &MainWindow::login_to_reset);
    ui_status_ = LOGIN_UI;
}

// 从登录界面切换到重置密码界面的槽函数
void MainWindow::login_to_reset() {
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    reset_dialog_ = new ResetDialog(this);
    reset_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(reset_dialog_);

    login_dialog_->hide();
    reset_dialog_->show();
    //注册返回登录信号和槽函数
    connect(reset_dialog_, &ResetDialog::reset_switch_login, this, &MainWindow::reset_to_login);
    ui_status_ = RESET_UI;
}

//从重置界面返回登录界面
void MainWindow::reset_to_login() {
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    login_dialog_ = new LoginDialog(this);
    login_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(login_dialog_);

    reset_dialog_->hide();
    login_dialog_->show();
    //连接登录界面忘记密码信号
    connect(login_dialog_, &LoginDialog::switch_reset, this, &MainWindow::login_to_reset);
    //连接登录界面注册信号
    connect(login_dialog_, &LoginDialog::switch_sign_up, this, &MainWindow::login_to_sign_up);
    ui_status_ = RESET_UI;
}

// 登录成功后跳转到聊天界面槽函数
void MainWindow::login_to_chat() {
    chat_dialog_ = new ChatDialog();
    chat_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(chat_dialog_);
    chat_dialog_->show();
    login_dialog_->hide();
    this->setMinimumSize(QSize(1050, 900));
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui_status_ = CHAT_UI;
    // 加载聊天列表
    chat_dialog_->loadChatList();
}

// 下线槽函数
void MainWindow::slotOffline() {
    // 使用静态方法直接弹出一个信息框
    QMessageBox::information(this, "下线提示", "同账号异地登录，该终端下线！");
    TcpMgr::getInstance()->closeConnection();
    offlineLogin();
}

void MainWindow::offlineLogin() {
    if (ui_status_ == LOGIN_UI) {
        return;
    }
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    login_dialog_ = new LoginDialog(this);
    login_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(login_dialog_);

    chat_dialog_->hide();
    this->setMaximumSize(300, 500);
    this->setMinimumSize(300, 500);
    this->resize(300, 500);
    login_dialog_->show();
    //连接登录界面忘记密码信号
    connect(login_dialog_, &LoginDialog::switch_reset, this, &MainWindow::login_to_reset);
    //连接登录界面注册信号
    connect(login_dialog_, &LoginDialog::switch_sign_up, this, &MainWindow::login_to_sign_up);
    ui_status_ = LOGIN_UI;
}

void MainWindow::slotExcepConOffline() {
    // 使用静态方法直接弹出一个信息框
    QMessageBox::information(this, "下线提示", "心跳超时或临界异常，该终端下线！");
    TcpMgr::getInstance()->closeConnection();
    offlineLogin();
}

