#pragma once

#include <QDialog>
#include "ui_resetdialog.h"

/******************************************************************************
 * @file       resetdialog.h
 * @brief      重置密码模块类
 *
 * @author     lueying
 * @date       2026/1/1
 * @history
 *****************************************************************************/

class ResetDialog : public QDialog {
	Q_OBJECT
signals:
	// 重置界面切换到登录界面
	void reset_switch_login();

public:
	ResetDialog(QWidget *parent = nullptr);
	~ResetDialog();	
	// 添加错误提示信息
	void addTipErr(TipErr te, QString tips);
	// 删除错误提示信息
	void delTipErr(TipErr te);

public slots:
	// 获取验证码按钮点击槽函数
	void get_verify_btn_clicked();
	// 重置确认按钮点击槽函数
	void confirm_btn_clicked();
	// 重置模块http请求接受槽函数
	void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
	// 取消按钮槽点击槽函数
	void cancel_btn_clicked();

private:
	Ui::ResetDialogClass* ui;
	QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;	// http请求与接受处理函数映射表
	void initHttpHandlers();				// 注册消息处理

	QMap<TipErr, QString> _tip_errs;		// 提示错误映射表
	void showTip(QString str, bool b_ok);	// 显示提示信息
	// 错误检测
	bool checkUserNameValid();
	bool checkPwdValid();
	bool checkEmailValid();
	bool checkVerifyCodeValid();
};

