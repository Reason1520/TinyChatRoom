#include "signupdialog.h"
#include "global.h"
#include "httpmgr.h"
#include <QRegularExpression>
#include <qurl.h>

/******************************************************************************
 * @file       signupdialog.h
 * @brief      注册模块类实现
 *
 * @author     lueying
 * @date       2025/12/7
 * @history
 *****************************************************************************/

SignUpDialog::SignUpDialog(QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::SignUpClass) {
	ui->setupUi(this);
	// 设置密码输入框的echoMode为密码
	ui->pw_edit->setEchoMode(QLineEdit::Password);
	ui->pw_confirm_edit->setEchoMode(QLineEdit::Password);

	// 初始化状态
	ui->err_tip->setProperty("state", "normal");
	ui->err_tip->clear(); // 清空错误提示

	initHttpHandlers();

	// 连接获取验证码按钮点击事件
	connect(ui->get_verify_btn, &QPushButton::clicked, this, &SignUpDialog::get_verify_btn_clicked);
	// 连接确认按钮点击事件
	connect(ui->comfirm_btn, &QPushButton::clicked, this, &SignUpDialog::confirm_btn_clicked);
	// 连接取消按钮点击事件
	connect(ui->cancel_btn, &QPushButton::clicked, this, &SignUpDialog::cancel_btn_clicked);
	// 连接返回按钮点击事件
	connect(ui->return_btn, &QPushButton::clicked, this, &SignUpDialog::return_btn_clicked);

	// 连接httpmgr的信号槽
	connect(HttpMgr::getInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &SignUpDialog::slot_reg_mod_finish);

	// 输入框编辑完成后的校验逻辑
	connect(ui->user_name_edit, &QLineEdit::editingFinished, this, [this]() {
		checkUserNameValid();
	});

	connect(ui->email_edit, &QLineEdit::editingFinished, this, [this]() {
		checkEmailValid();
	});

	connect(ui->pw_edit, &QLineEdit::editingFinished, this, [this]() {
		checkPwdValid();
	});

	connect(ui->pw_confirm_edit, &QLineEdit::editingFinished, this, [this]() {

	});

	connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this]() {
		checkVerifyCodeValid();
	});

	// 设置密码显示
	// 设置浮动显示手形状
	ui->pass_visible->setCursor(Qt::PointingHandCursor);
	ui->confirm_visible->setCursor(Qt::PointingHandCursor);

	ui->pass_visible->setState("unvisible", "unvisible_hover", "", "visible",
		"visible_hover", "");

	ui->confirm_visible->setState("unvisible", "unvisible_hover", "", "visible",
		"visible_hover", "");
	
	// 连接密码隐藏点击事件
	connect(ui->pass_visible, &ClickedLabel::clicked, this, [this]() {
		auto state = ui->pass_visible->getCurState();
		if (state == ClickLbState::Normal) {
			ui->pw_edit->setEchoMode(QLineEdit::Password);
		}
		else {
			ui->pw_edit->setEchoMode(QLineEdit::Normal);
		}
		qDebug() << "Label was clicked!";
	});

	connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
		auto state = ui->confirm_visible->getCurState();
		if (state == ClickLbState::Normal) {
			ui->pw_confirm_edit->setEchoMode(QLineEdit::Password);
		}
		else {
			ui->pw_confirm_edit->setEchoMode(QLineEdit::Normal);
		}
		qDebug() << "Label was clicked!";
	});

	// 创建定时器
	countdown_timer_ = new QTimer(this);
	countdown_ = 5;
	// 连接信号和槽
	connect(countdown_timer_, &QTimer::timeout, [this]() {
		if (countdown_ == 0) {
			countdown_timer_->stop();
			emit sign_up_switch_login();
			return;
		}
		countdown_--;
		auto str = QString("注册成功，%1 s后返回登录").arg(countdown_);
		ui->tip1_lb->setText(str);
	});
}

SignUpDialog::~SignUpDialog()
{
	if (ui) {
		delete ui;
	}
}

// 获取验证码按钮点击槽函数
void SignUpDialog::get_verify_btn_clicked()
{
	//验证邮箱的地址正则表达式
	auto email = ui->email_edit->text();
	// 邮箱地址的正则表达式
	QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
	bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
	if (match) {
		//发送http请求获取验证码
		QJsonObject json_obj;
		json_obj["email"] = email;
		HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/get_verifycode"),
			json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);
	}
	else {
		//提示邮箱不正确
		showTip(tr("邮箱地址不正确"), false);
	}
}

// 确认按钮点击槽函数
void SignUpDialog::confirm_btn_clicked() {
	bool valid = checkUserNameValid();
	if (!valid) {
		return;
	}

	valid = checkEmailValid();
	if (!valid) {
		return;
	}

	valid = checkPwdValid();
	if (!valid) {
		return;
	}

	valid = checkPwdConfirmValid();
	if (!valid) {
		return;
	}

	valid = checkVerifyCodeValid();
	if (!valid) {
		return;
	}

	// 发送http请求注册用户
	QJsonObject json_obj;
	json_obj["user"] = ui->user_name_edit->text();
	json_obj["email"] = ui->email_edit->text();
	json_obj["passwd"] = xorString(ui->pw_edit->text());
	json_obj["confirm"] = xorString(ui->pw_confirm_edit->text());
	json_obj["varifycode"] = ui->verify_edit->text();
	HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/user_register"),
		json_obj, ReqId::ID_REG_USER, Modules::REGISTERMOD);
}

// 返回按钮槽点击函数
void SignUpDialog::return_btn_clicked() {
	countdown_timer_->stop();
	emit sign_up_switch_login();
}

// 取消按钮点击槽函数
void SignUpDialog::cancel_btn_clicked() {
	countdown_timer_->stop();
	emit sign_up_switch_login();
}

// 显示提示信息
void SignUpDialog::showTip(QString str, bool b_ok) {
	if (b_ok) {
		ui->err_tip->setProperty("state", "normal");
	}
	else {
		ui->err_tip->setProperty("state", "err");
	}

	ui->err_tip->setText(str);

	repolish(ui->err_tip);
}

// 注册模块http请求接受槽函数
void SignUpDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err) {
	if (err != ErrorCodes::SUCCESS) {
		showTip(tr("网络请求错误"), false);
		return;
	}

	// 解析 JSON 字符串,res需转化为QByteArray
	QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
	//json解析错误
	if (jsonDoc.isNull()) {
		showTip(tr("json解析错误"), false);
		return;
	}

	//json解析错误
	if (!jsonDoc.isObject()) {
		showTip(tr("json解析错误"), false);
		return;
	}

	QJsonObject jsonObj = jsonDoc.object();

	//调用对应的逻辑，根据id回调
	handlers_[id](jsonDoc.object());

	return;
}

// 注册消息处理
void SignUpDialog::initHttpHandlers() {
	//注册获取验证码回包逻辑
	handlers_.insert(ReqId::ID_GET_VARIFY_CODE, [this](QJsonObject jsonObj) {
		int error = jsonObj["error"].toInt();
		if (error != ErrorCodes::SUCCESS) {
			showTip(tr("参数错误"), false);
			return;
		}
		auto email = jsonObj["email"].toString();
		showTip(tr("验证码已发送到邮箱，注意查收"), true);
		qDebug() << "email is " << email;
	});

	//注册注册用户回包逻辑
	handlers_.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj) {
		int error = jsonObj["error"].toInt();
		if (error != ErrorCodes::SUCCESS) {
			showTip(tr("参数错误"), false);
			return;
		}
		auto email = jsonObj["email"].toString();
		showTip(tr("用户注册成功"), true);
		qDebug() << "email is " << email;
		qDebug() << "user uuid is " << jsonObj["uuid"].toString();
		changeTipPage();
	});
}

// 添加错误信息
void SignUpDialog::addTipErr(TipErr te, QString tips) {
	tip_errs_[te] = tips;
	showTip(tips, false);
}

// 删除错误信息
void SignUpDialog::delTipErr(TipErr te) {
	tip_errs_.remove(te);
	if (tip_errs_.empty()) {
		ui->err_tip->clear();
		return;
	}

	showTip(tip_errs_.first(), false);
}


// 错误监测
bool SignUpDialog::checkUserNameValid() {
	if (ui->user_name_edit->text() == "") {
		addTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
		return false;
	}

	delTipErr(TipErr::TIP_USER_ERR);
	return true;
}


bool SignUpDialog::checkPwdValid() {
	auto pass = ui->pw_edit->text();

	if (pass.length() < 6 || pass.length() > 15) {
		//提示长度不准确
		addTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
		return false;
	}

	// 创建一个正则表达式对象，按照上述密码要求
	// 这个正则表达式解释：
	// ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
	QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
	bool match = regExp.match(pass).hasMatch();
	if (!match) {
		//提示字符非法
		addTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
		return false;
	}

	delTipErr(TipErr::TIP_PWD_ERR);

	return true;
}

bool SignUpDialog::checkPwdConfirmValid() {
	if (ui->pw_edit->text() != ui->pw_confirm_edit->text()) {
		//提示密码确认不正确
		addTipErr(TipErr::TIP_CONFIRM_ERR, tr("两次密码输入不一致"));
		return false;
	}

	delTipErr(TipErr::TIP_CONFIRM_ERR);

	return true;
}


bool SignUpDialog::checkEmailValid()
{
	//验证邮箱的地址正则表达式
	auto email = ui->email_edit->text();
	// 邮箱地址的正则表达式
	QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
	bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
	if (!match) {
		//提示邮箱不正确
		addTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
		return false;
	}

	delTipErr(TipErr::TIP_EMAIL_ERR);
	return true;
}

bool SignUpDialog::checkVerifyCodeValid()
{
	auto pass = ui->verify_edit->text();
	if (pass.isEmpty()) {
		addTipErr(TipErr::TIP_VARIFY_ERR, tr("验证码不能为空"));
		return false;
	}

	delTipErr(TipErr::TIP_VARIFY_ERR);
	return true;
}

// 切换到提示页面
void SignUpDialog::changeTipPage()
{
	countdown_timer_->stop();
	ui->stackedWidget->setCurrentWidget(ui->page_9);

	// 启动定时器，设置间隔为1000毫秒（1秒）
	countdown_timer_->start(1000);
}