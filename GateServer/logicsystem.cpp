#include "logicsystem.h"
#include "httpconnection.h"
#include "verifygrpcclient.h"
#include "statusgrpcclient.h"
#include "redismgr.h"
#include "mysqlmgr.h"
#include "const.h"
#include <iostream>

/******************************************************************************
 * @file       logicsystem.cpp
 * @brief      处理http请求的逻辑系统类实现
 *
 * @author     lueying
 * @date       2025/12/14
 * @history
 *****************************************************************************/

LogicSystem::LogicSystem() {
    regGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->response_.body()) << "receive get_test req " << std::endl;
        int i = 0;
        for (auto& elem : connection->get_params_) {
            i++;
            beast::ostream(connection->response_.body()) << "param" << i << " key is " << elem.first;
            beast::ostream(connection->response_.body()) << ", " << " value is " << elem.second << std::endl;
        }
    });

    // 验证码获取回调逻辑
    regPost("/get_verifycode", [](std::shared_ptr<HttpConnection> connection) {
        // 将请求体（buffer序列）转化为string
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;       // 返回给客户端的JSON
        Json::Reader reader;    // 用于解析客户端发来的JSON
        Json::Value src_root;   // 解析后的客户端数据的JSON
        // 将string解析为JSON
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            // 如果解析失败，返回错误信息
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        auto email = src_root["email"].asString();
        // 调用grpc接口获取验证码
        GetVerifyRsp rsp = VerifyGrpcClient::getInstance()->getVerifyCode(email);
        std::cout << "email is " << email << std::endl;
        root["error"] = 0;
        root["email"] = src_root["email"];
        // 将JSON转化为string
        std::string jsonstr = root.toStyledString();
        // 将string写入response body
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });

    // 用户注册逻辑
    regPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        auto email = src_root["email"].asString();
        auto name = src_root["user"].asString();
        auto pwd = src_root["passwd"].asString();
        auto confirm = src_root["confirm"].asString();

        if (pwd != confirm) {
            std::cout << "password err " << std::endl;
            root["error"] = ErrorCodes::PwdConfirmErr;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        //先查找redis中email对应的验证码是否合理
        std::string  verify_code;
        std::string redis_key = CODEPREFIX + email;
        std::cout << "Attempting to find Redis Key: [" << redis_key << "]" << std::endl;
        bool b_get_verify = RedisMgr::getInstance()->get(redis_key, verify_code);
        if (!b_get_verify) {
            std::cout << " get varify code expired" << std::endl;
            std::cout << "Redis Error: Key NOT FOUND!" << std::endl;
            root["error"] = ErrorCodes::VerifyExpired;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        } else {
            std::cout << "Redis Found: [" << verify_code << "], User Input: [" << src_root["varifycode"].asString() << "]" << std::endl;
        }

        if (verify_code != src_root["varifycode"].asString()) {
            std::cout << " varify code error" << std::endl;
            root["error"] = ErrorCodes::VerifyCodeErr;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        //查找数据库判断用户是否存在
        int uid = MysqlMgr::getInstance()->regUser(name, email, pwd);
        if (uid == 0 || uid == -1) {
            std::cout << " user or email exist" << std::endl;
            root["error"] = ErrorCodes::UserExist;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }
        root["error"] = 0;
        root["uid"] = uid;
        root["email"] = email;
        root["user"] = name;
        root["passwd"] = pwd;
        root["confirm"] = confirm;
        root["varifycode"] = src_root["varifycode"].asString();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });

    //重置密码回调逻辑
    regPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        auto email = src_root["email"].asString();
        auto name = src_root["user"].asString();
        auto pwd = src_root["passwd"].asString();

        //先查找redis中email对应的验证码是否合理
        std::string  varify_code;
        bool b_get_varify = RedisMgr::getInstance()->get(CODEPREFIX + src_root["email"].asString(), varify_code);
        if (!b_get_varify) {
            std::cout << " get varify code expired" << std::endl;
            root["error"] = ErrorCodes::VerifyExpired;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        if (varify_code != src_root["varifycode"].asString()) {
            std::cout << " varify code error" << std::endl;
            root["error"] = ErrorCodes::VerifyCodeErr;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        //查询数据库判断用户名和邮箱是否匹配
        bool email_valid = MysqlMgr::getInstance()->checkEmail(name, email);
        if (!email_valid) {
            std::cout << " user email not match" << std::endl;
            root["error"] = ErrorCodes::EmailNotMatch;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        //更新密码为最新密码
        bool b_up = MysqlMgr::getInstance()->updatePwd(name, pwd);
        if (!b_up) {
            std::cout << " update pwd failed" << std::endl;
            root["error"] = ErrorCodes::PwdUpdateFailed;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        std::cout << "succeed to update password" << pwd << std::endl;
        root["error"] = 0;
        root["email"] = email;
        root["user"] = name;
        root["passwd"] = pwd;
        root["varifycode"] = src_root["varifycode"].asString();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });

    //用户登录逻辑
    regPost("/user_login", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        auto name = src_root["user"].asString();
        auto pwd = src_root["passwd"].asString();
        UserInfo userInfo;
        //查询数据库判断用户名和密码是否匹配
        bool pwd_valid = MysqlMgr::getInstance()->checkPwd(name, pwd, userInfo);
        if (!pwd_valid) {
            std::cout << " user pwd not match" << std::endl;
            root["error"] = ErrorCodes::PasswdInvalid;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        //查询StatusServer找到合适的连接
        auto reply = StatusGrpcClient::getInstance()->getChatServer(userInfo.uid);
        if (reply.error()) {
            std::cout << " grpc get chat server failed, error is " << reply.error() << std::endl;
            root["error"] = ErrorCodes::RPCGetFailed;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }

        std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
        root["error"] = 0;
        root["user"] = name;
        root["uid"] = userInfo.uid;
        root["token"] = reply.token();
        root["host"] = reply.host();
        root["port"] = reply.port();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });
}

LogicSystem::~LogicSystem() 
{

}

// 注册get请求的回调函数
void LogicSystem::regGet(std::string url, HttpHandler handler) {
    get_handlers_.insert(make_pair(url, handler));
}

// 注册post请求的回调函数
void LogicSystem::regPost(std::string url, HttpHandler handler) {
    post_handlers_.insert(make_pair(url, handler));
}

// get请求处理函数
bool LogicSystem::handleGet(std::string path, std::shared_ptr<HttpConnection> con) {
    if (get_handlers_.find(path) == get_handlers_.end()) {
        return false;
    }

    get_handlers_[path](con);
    return true;
}

// post请求处理函数
bool LogicSystem::handlePost(std::string path, std::shared_ptr<HttpConnection> con) {
    if (post_handlers_.find(path) == post_handlers_.end()) {
        return false;
    }

    post_handlers_[path](con);
    return true;
}
