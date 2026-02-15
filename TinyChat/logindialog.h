#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "ui_logindialog.h"
#include "global.h"
/******************************************************************************
 * @file       logindialog.h
 * @brief      登录对话框类
 *
 * @author     lueying
 * @date       2025/12/7
 * @history
 *****************************************************************************/

class LoginDialog : public QDialog
{
	Q_OBJECT

signals:
	void switch_sign_up();		// 切换到注册页面的信号
	void switch_reset();		// 切换到重置密码页面的信号
	void sig_connect_tcp(std::shared_ptr<ServerInfo>);			// 通知TcpMgr建立连接
	void sig_connect_res_server(std::shared_ptr<ServerInfo>);	// 通知FlieTcpMgr建立连接

public:
	LoginDialog(QWidget *parent = nullptr);
	~LoginDialog();

public slots:
	void on_login_button_clicked();	// 登录按钮点击槽函数
	void on_forget_button_clicked();	// 忘记密码按钮点击槽函数
	// 登录模块http请求接受槽函数
	void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
	// 处理TCP连接结果的槽函数
	void slot_tcp_con_finish(bool bsuccess);
	// 登录聊天失败槽函数
	void slot_login_failed(int err);
	// 资源服务器连接结果槽函数
	void slot_res_con_finish(bool bsuccess);

private:
	Ui::LoginDialogClass *ui;
	// server信息存储
	std::shared_ptr<ServerInfo> si_;
	QMap<ReqId, std::function<void(const QJsonObject&)>> handlers_;	// http请求与接受处理函数映射表
	QMap<TipErr, QString> tip_errs_;		// 提示错误映射表

	void initHead();						// 初始化头像
	void initHttpHandlers();				// 注册消息处理
	void showTip(QString str, bool b_ok);	// 显示提示信息
	// 检测函数
	bool checkUserValid();
	bool checkPwdValid();
	// UI控制函数：控制按钮状态
	void enableBtn(bool enabled) {
		ui->login_btn->setEnabled(enabled);
		ui->signup_btn->setEnabled(enabled);
	}
	// 添加错误信息
	void addTipErr(TipErr te, QString tips);
	// 删除错误信息
	void delTipErr(TipErr te);
};

#endif // LOGINDIALOG_H

