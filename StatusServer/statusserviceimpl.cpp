#include "statusserviceimpl.h"
#include "configmgr.h"
#include "const.h"
#include "redismgr.h"
#include <boost/uuid/uuid.hpp>            // uuid 类定义
#include <boost/uuid/uuid_generators.hpp> // random_generator 定义
#include <boost/uuid/uuid_io.hpp>         // 关键：提供对 to_string 的支持

/******************************************************************************
 * @file       statusserviceimpl.cpp
 * @brief      状态服务类实现
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

StatusServiceImpl::StatusServiceImpl() {
    auto& cfg = ConfigMgr::getInst();
    auto server_list = cfg["chatservers"]["Name"];

    std::vector<std::string> words;

    std::stringstream ss(server_list);
    std::string word;

    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }

    for (auto& word : words) {
        if (cfg[word]["Name"].empty()) {
            continue;
        }

        ChatServer server;
        server.port = cfg[word]["Port"];
        server.host = cfg[word]["Host"];
        server.name = cfg[word]["Name"];
        servers_[server.name] = server;
    }
}

std::string generate_unique_string() {
    // 创建UUID对象（全局唯一标识符）
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    // 将UUID转换为字符串
    std::string unique_string = to_string(uuid);

    return unique_string;
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply) {
    std::string prefix("llfc status server has received :  ");
    const auto& server = getAvailableServer();
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    insertToken(request->uid(), reply->token());
    return Status::OK;
}

// 获取最小连接数的chatserver名字
ChatServer StatusServiceImpl::getAvailableServer() {
    std::lock_guard<std::mutex> guard(servers_mutex_);
    auto minServer = servers_.begin()->second;

    auto count_str = RedisMgr::getInstance()->hGet(LOGIN_COUNT, minServer.name);
    if (count_str.empty()) {
        //不存在则默认设置为最大
        minServer.con_count = INT_MAX;
    }
    else {
        minServer.con_count = std::stoi(count_str);
    }


    // 使用范围基于for循环
    for (auto& server : servers_) {

        if (server.second.name == minServer.name) {
            continue;
        }

        auto count_str = RedisMgr::getInstance()->hGet(LOGIN_COUNT, server.second.name);
        if (count_str.empty()) {
            server.second.con_count = INT_MAX;
        }
        else {
            server.second.con_count = std::stoi(count_str);
        }

        if (server.second.con_count < minServer.con_count) {
            minServer = server.second;
        }
    }

    return minServer;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
    auto uid = request->uid();
    auto token = request->token();
    std::lock_guard<std::mutex> guard(tokens_mutex_);
    auto iter = tokens_.find(uid);
    if (iter == tokens_.end()) {
        reply->set_error(ErrorCodes::UidInvalid);
        return Status::OK;
    }
    if (iter->second != token) {
        reply->set_error(ErrorCodes::TokenInvalid);
        return Status::OK;
    }
    reply->set_error(ErrorCodes::Success);
    reply->set_uid(uid);
    reply->set_token(token);
    return Status::OK;
}

void StatusServiceImpl::insertToken(int uid, std::string token) {
    std::string uid_str = std::to_string(uid);
    std::string token_key = USERTOKENPREFIX + uid_str;
    RedisMgr::getInstance()->set(token_key, token);
}