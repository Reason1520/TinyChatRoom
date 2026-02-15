#include "logindialog.h"
#include "httpmgr.h"
#include "tcpmgr.h"
#include "filetcpmgr.h"
#include <QPainter>
#include <QPainterPath>

/******************************************************************************
 * @file       logindialog.cpp
 * @brief      登录对话框类实现文件
 *
 * @author     lueying
 * @date       2025/12/7
 * @history
 *****************************************************************************/

LoginDialog::LoginDialog(QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::LoginDialogClass) {
	ui->setupUi(this);
	// 注册按钮
	connect(ui->signup_btn, &QPushButton::clicked, this, &LoginDialog::switch_sign_up);
	// 忘记密码按钮
	connect(ui->forget_btn, &QPushButton::clicked, this, &LoginDialog::on_forget_button_clicked);
    // 登录按钮
    connect(ui->login_btn, &QPushButton::clicked, this, &LoginDialog::on_login_button_clicked);

    initHttpHandlers();
    //连接登录回包信号
    connect(HttpMgr::getInstance().get(), &HttpMgr::sig_login_mod_finish, this,
        &LoginDialog::slot_login_mod_finish);
    //连接tcp连接请求的信号和槽函数
    connect(this, &LoginDialog::sig_connect_tcp, TcpMgr::getInstance().get(), &TcpMgr::slot_tcp_connect);
    //连接tcp管理者发出的连接成功信号
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_con_success, this, &LoginDialog::slot_tcp_con_finish);
    //连接tcp管理者发出的登陆失败信号
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_login_failed, this, &LoginDialog::slot_login_failed);

    //连接tcp连接资源服务器请求的信号和槽函数
    connect(this, &LoginDialog::sig_connect_res_server,
        FileTcpMgr::getInstance().get(), &FileTcpMgr::slot_tcp_connect);

    //连接资源管理tcp发出的连接成功信号
    connect(FileTcpMgr::getInstance().get(), &FileTcpMgr::sig_con_success, this, &LoginDialog::slot_res_con_finish);
    initHead();
}

LoginDialog::~LoginDialog() {
	if (ui) {
		delete ui;
	}
}

// 初始化头像
void LoginDialog::initHead() {
    // 定义一个固定的目标尺寸
    QSize targetSize(200, 200);
    // 强制设置 Label 的大小，防止被布局压缩
    ui->head_label->setFixedSize(targetSize);

    // 加载图片
    QPixmap originalPixmap(":/image/res/QQ图片20221122234905.png");
    // 设置图片自动缩放
    qDebug() << originalPixmap.size() << ui->head_label->size();
    originalPixmap = originalPixmap.scaled(ui->head_label->size(),
        Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个和原始图片相同大小的QPixmap，用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent); // 用透明色填充

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing); // 设置抗锯齿，使圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 使用QPainterPath设置圆角
    QPainterPath path;
    path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10, 10); // 最后两个参数分别是x和y方向的圆角半径
    painter.setClipPath(path);

    // 将原始图片绘制到roundedPixmap上
    painter.drawPixmap(0, 0, originalPixmap);

    // 设置绘制好的圆角图片到QLabel上
    ui->head_label->setPixmap(roundedPixmap);

}

// http消息处理
void LoginDialog::initHttpHandlers() {
    //注册获取登录回包逻辑
    handlers_.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            enableBtn(true);
            return;
        }
        auto email = jsonObj["email"].toString();

        //发送信号通知tcpMgr发送长链接
        si_ = std::make_shared<ServerInfo>();

        si_->_uid = jsonObj["uid"].toInt();
        si_->_chat_host = jsonObj["chathost"].toString();
        si_->_chat_port = jsonObj["chatport"].toString();
        si_->_token = jsonObj["token"].toString();

        si_->_res_host = jsonObj["reshost"].toString();
        si_->_res_port = jsonObj["resport"].toString();


        qDebug() << "email is " << email << " uid is " << si_->_uid << " chat host is "
            << si_->_chat_host << " chat port is "
            << si_->_chat_port << " token is " << si_->_token
            << " res host is " << si_->_res_host
            << " res port is " << si_->_res_port;
        emit sig_connect_tcp(si_);
        // qDebug() << "send thread is " << QThread::currentThread();
        // emit sig_test();
        });
}

// 登录按钮点击槽函数
void LoginDialog::on_login_button_clicked() {
    qDebug() << "login btn clicked";
    if (checkUserValid() == false) {
        return;
    }

    if (checkPwdValid() == false) {
        return;
    }

    auto user = ui->user_name_edit->text();
    auto pwd = ui->pw_edit->text();
    //发送http请求登录
    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/user_login"),
        json_obj, ReqId::ID_LOGIN_USER, Modules::LOGINMOD);
}

// 忘记密码按钮点击槽函数
void LoginDialog::on_forget_button_clicked() {
	qDebug() << "slot forget pwd";
	emit switch_reset();
}

// 登录模块http请求接受槽函数
void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err) {
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
    handlers_[id](jsonDoc.object());

    return;
}

// 收到tcp连接结果槽函数
void LoginDialog::slot_tcp_con_finish(bool bsuccess)
{
    if (bsuccess) {
        showTip(tr("聊天服务连接成功，正在连接资源服务器..."), true);
        emit sig_connect_res_server(si_);
    }
    else {
        showTip(tr("网络异常"), false);
    }
}

// 登录聊天失败槽函数
void LoginDialog::slot_login_failed(int err) {
    QString result = QString("登录失败, err is %1")
        .arg(err);
    showTip(result, false);
    enableBtn(true);
}

// 资源服务器连接结果槽函数
void LoginDialog::slot_res_con_finish(bool bsuccess) {
    if (bsuccess) {
        showTip(tr("聊天服务连接成功，正在登录..."), true);
        QJsonObject jsonObj;
        jsonObj["uid"] = si_->_uid;
        jsonObj["token"] = si_->_token;

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

        //发送tcp请求给chat server
        emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN, jsonData);

    }
    else {
        showTip(tr("网络异常"), false);
        enableBtn(true);
    }
}

// 添加错误信息
void  LoginDialog::addTipErr(TipErr te, QString tips) {
    tip_errs_[te] = tips;
    showTip(tips, false);
}

// 删除错误信息
void  LoginDialog::delTipErr(TipErr te) {
    tip_errs_.remove(te);
    if (tip_errs_.empty()) {
        ui->err_tip->clear();
        return;
    }

    showTip(tip_errs_.first(), false);
}

// 显示提示信息
void LoginDialog::showTip(QString str, bool b_ok) {
    if (b_ok) {
        ui->err_tip->setProperty("state", "normal");
    }
    else {
        ui->err_tip->setProperty("state", "err");
    }

    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

// 检测函数
bool LoginDialog::checkUserValid() {
    if (ui->user_name_edit->text() == "") {
        addTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }

    delTipErr(TipErr::TIP_USER_ERR);

    return true;
}

bool LoginDialog::checkPwdValid() {
    auto pass = ui->pw_edit->text();

    if (pass.length() < 6 || pass.length() > 15) {
        //提示长度不准确
        addTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    delTipErr(TipErr::TIP_PWD_ERR);

    return true;
}

