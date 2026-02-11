#ifndef SIGNUP_H
#define SIGNUP_H

#include <QDialog>
#include "ui_signup.h"
#include "global.h"

/******************************************************************************
 * @file       signupdialog.h
 * @brief      注册模块类
 *
 * @author     lueying
 * @date       2025/12/7
 * @history
 *****************************************************************************/

class SignUpDialog : public QDialog {
	Q_OBJECT

signals:
	// 注册界面切换到登录界面
	void sign_up_switch_login();

public:
	SignUpDialog(QWidget *parent = nullptr);
	~SignUpDialog();
	// 添加错误信息
	void addTipErr(TipErr te, QString tips);
	// 删除错误信息
	void delTipErr(TipErr te);
	// 切换到提示界面
	void changeTipPage();

public slots:
	// 获取验证码按钮点击槽函数
	void get_verify_btn_clicked();
	// 注册确认按钮点击槽函数
	void confirm_btn_clicked();
	// 注册模块http请求接受槽函数
	void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
	// 返回按钮点击槽函数
	void return_btn_clicked();
	// 取消按钮槽点击函数
	void cancel_btn_clicked();

private:
	Ui::SignUpClass* ui;
	QMap<ReqId, std::function<void(const QJsonObject&)>> handlers_;	// http请求与接受处理函数映射表
	void showTip(QString str, bool b_ok);	// 显示提示信息
	void initHttpHandlers();				// 注册消息处理
	QMap<TipErr, QString> tip_errs_;		// 提示错误映射表
	QTimer* countdown_timer_;				// 提示界面定时器
	int countdown_;							// 定时器倒数秒数
	// 错误检测
	bool checkUserNameValid();
	bool checkPwdValid();
	bool checkPwdConfirmValid();
	bool checkEmailValid();
	bool checkVerifyCodeValid();

};

#endif // SIGNUP_H