#include "statusgrpcclient.h"
#include "configmgr.h"
#include <queue>
#include <atomic>

/******************************************************************************
 * @file       statusgrpcclient.cpp
 * @brief      状态服务的gRPC客户端类实现
 *
 * @author     lueying
 * @date       2026/1/3
 * @history
 *****************************************************************************/

// 连接池
class StatusConPool {
public:
    StatusConPool(size_t poolSize, std::string host, std::string port)
        : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
        for (size_t i = 0; i < poolSize_; ++i) {

            std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
                grpc::InsecureChannelCredentials());

            connections_.push(StatusService::NewStub(channel));
        }
    }

    ~StatusConPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        close();
        while (!connections_.empty()) {
            connections_.pop();
        }
    }

    std::unique_ptr<StatusService::Stub> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] {
            if (b_stop_) {
                return true;
            }
            return !connections_.empty();
            });
        //如果停止则直接返回空指针
        if (b_stop_) {
            return  nullptr;
        }
        auto context = std::move(connections_.front());
        connections_.pop();
        return context;
    }

    void returnConnection(std::unique_ptr<StatusService::Stub> context) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_) {
            return;
        }
        connections_.push(std::move(context));
        cond_.notify_one();
    }

    void close() {
        b_stop_ = true;
        cond_.notify_all();
    }

private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<StatusService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

// RAII 类
class StatusConGuard {
public:
    // 构造时借出连接
    StatusConGuard(std::unique_ptr<StatusConPool>& pool) : pool_(pool) {
        if (pool_) {
            con_ = pool_->getConnection();
        }
    }

    // 析构时自动归还连接
    ~StatusConGuard() {
        if (con_ && pool_) {
            pool_->returnConnection(std::move(con_));
        }
    }

    // 获取内部连接指针
    StatusService::Stub* get() {
        return con_.get();
    }

    // 禁用拷贝，确保连接归还逻辑唯一
    StatusConGuard(const StatusConGuard&) = delete;
    StatusConGuard& operator=(const StatusConGuard&) = delete;

private:
    std::unique_ptr<StatusConPool>& pool_;
    std::unique_ptr<StatusService::Stub> con_;
};

// 获取聊天服务器信息
GetChatServerRsp StatusGrpcClient::getChatServer(int uid) {
    ClientContext context;
    GetChatServerRsp reply;
    GetChatServerReq request;
    request.set_uid(uid);
    StatusConGuard guard(pool_);
    auto stub = guard.get();
    Status status = stub->GetChatServer(&context, request, &reply);
    if (status.ok()) {
        return reply;
    }
    else {
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}

StatusGrpcClient::StatusGrpcClient() {
    auto& gCfgMgr = ConfigMgr::getInst();
    std::string host = gCfgMgr["StatusServer"]["Host"];
    std::string port = gCfgMgr["StatusServer"]["Port"];
    pool_.reset(new StatusConPool(5, host, port));
}

StatusGrpcClient::~StatusGrpcClient() {
    if (pool_) {
        pool_->close();
    }
}