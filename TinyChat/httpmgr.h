#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include "global.h"
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>

/******************************************************************************
 * @file       HttpMgr.h
 * @brief      Http管理器类
 *
 * @author     lueying
 * @date       2025/12/8
 * @history
 *****************************************************************************/

class HttpMgr :public QObject, public Singleton<HttpMgr>,
    public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT

public:
    ~HttpMgr();
    // 发送http POST请求
    void postHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);
public slots:
    // 接受http请求完成信号的槽函数
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
private:
    friend class Singleton<HttpMgr>;
    HttpMgr();
    QNetworkAccessManager manager_; // Qt网络访问管理器
signals:
    // http请求完成信号
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    // 注册模块的信号
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    // 重置模块的信号
    void sig_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    // 登录模块的信号
    void sig_login_mod_finish(ReqId id, QString res, ErrorCodes err);
};