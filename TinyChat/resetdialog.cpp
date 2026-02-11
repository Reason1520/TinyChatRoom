#include "resetdialog.h"
#include "ui_resetdialog.h"
#include <QDebug>
#include <QRegularExpression>
#include "global.h"
#include "httpmgr.h"

/******************************************************************************
 * @file       resetdialog.h
 * @brief      重置密码模块类
 *
 * @author     lueying
 * @date       2026/1/1
 * @history
 *****************************************************************************/

ResetDialog::ResetDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ResetDialogClass) {
    ui->setupUi(this);

    // 连接获取验证码按钮点击事件
    connect(ui->get_verify_btn_2, &QPushButton::clicked, this, &ResetDialog::get_verify_btn_clicked);
    // 连接重置确认按钮点击事件
    connect(ui->comfirm_btn_2, &QPushButton::clicked, this, &ResetDialog::confirm_btn_clicked);
    // 连接取消按钮点击事件
    connect(ui->cancel_btn_2, &QPushButton::clicked, this, &ResetDialog::cancel_btn_clicked);

    connect(ui->user_name_edit_2, &QLineEdit::editingFinished, this, [this]() {
        checkUserNameValid();
    });

    connect(ui->email_edit_2, &QLineEdit::editingFinished, this, [this]() {
        checkEmailValid();
    });

    connect(ui->pw_edit_2, &QLineEdit::editingFinished, this, [this]() {
        checkPwdValid();
    });


    connect(ui->verify_edit_2, &QLineEdit::editingFinished, this, [this]() {
        checkVerifyCodeValid();
    });

    //连接reset相关信号和注册处理回调
    initHttpHandlers();
    connect(HttpMgr::getInstance().get(), &HttpMgr::sig_reset_mod_finish, this,
        &ResetDialog::slot_reset_mod_finish);
}

ResetDialog::~ResetDialog() {}

// 添加错误提示信息
void ResetDialog::addTipErr(TipErr te, QString tips) {
    _tip_errs[te] = tips;
    showTip(tips, false);
}

// 删除错误提示信息
void ResetDialog::delTipErr(TipErr te) {
    _tip_errs.remove(te);
    if (_tip_errs.empty()) {
        ui->err_tip_2->clear();
        return;
    }

    showTip(_tip_errs.first(), false);
}

// 显示提示信息
void ResetDialog::showTip(QString str, bool b_ok)
{
    if (b_ok) {
        ui->err_tip_2->setProperty("state", "normal");
    }
    else {
        ui->err_tip_2->setProperty("state", "err");
    }

    ui->err_tip_2->setText(str);

    repolish(ui->err_tip_2);
}

// 获取验证码按钮点击槽函数
void ResetDialog::get_verify_btn_clicked() {
    qDebug() << "receive varify btn clicked ";
    auto email = ui->email_edit_2->text();
    auto bcheck = checkEmailValid();
    if (!bcheck) {
        return;
    }

    //发送http请求获取验证码
    QJsonObject json_obj;
    json_obj["email"] = email;
    HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/get_verifycode"),
        json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::RESETMOD);
}

// 注册消息处理
void ResetDialog::initHttpHandlers() {
    //注册获取验证码回包逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](QJsonObject jsonObj) {
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
    _handlers.insert(ReqId::ID_RESET_PWD, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("重置成功,点击返回登录"), true);
        qDebug() << "email is " << email;
        qDebug() << "user uuid is " << jsonObj["uuid"].toString();
    });
}

// 重置模块http请求接受槽函数
void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
{
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

    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());

    return;
}

// 重置确认按钮点击槽函数
void ResetDialog::confirm_btn_clicked()
{
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

    valid = checkVerifyCodeValid();
    if (!valid) {
        return;
    }

    //发送http重置用户请求
    QJsonObject json_obj;
    json_obj["user"] = ui->user_name_edit_2->text();
    json_obj["email"] = ui->email_edit_2->text();
    json_obj["passwd"] = xorString(ui->pw_edit_2->text());
    json_obj["varifycode"] = ui->verify_edit_2->text();
    HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/reset_pwd"),
        json_obj, ReqId::ID_RESET_PWD, Modules::RESETMOD);
}

// 返回按钮槽点击槽函数
void ResetDialog::cancel_btn_clicked() {
    emit reset_switch_login();
}


// 错误检测
bool ResetDialog::checkUserNameValid()
{
    if (ui->user_name_edit_2->text() == "") {
        addTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }

    delTipErr(TipErr::TIP_USER_ERR);
    return true;
}


bool ResetDialog::checkPwdValid()
{
    auto pass = ui->pw_edit_2->text();

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
        return false;;
    }

    delTipErr(TipErr::TIP_PWD_ERR);

    return true;
}

bool ResetDialog::checkEmailValid()
{
    //验证邮箱的地址正则表达式
    auto email = ui->email_edit_2->text();
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

bool ResetDialog::checkVerifyCodeValid()
{
    auto pass = ui->verify_edit_2->text();
    if (pass.isEmpty()) {
        addTipErr(TipErr::TIP_VARIFY_ERR, tr("验证码不能为空"));
        return false;
    }

    delTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}