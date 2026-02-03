#include <iostream>
#include <queue>
#include <atomic>
#include "redismgr.h"
#include "configmgr.h"

/******************************************************************************
 * @file       redismgr.cpp
 * @brief      redis操作封装实现
 *
 * @author     lueying
 * @date       2025/12/29
 * @history
 *****************************************************************************/

// redis连接池
class RedisConPool {
public:
    RedisConPool(size_t poolSize, const char* host, int port, const char* pwd)
        : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
        for (size_t i = 0; i < poolSize_; ++i) {
            auto* context = redisConnect(host, port);
            if (context == nullptr || context->err != 0) {
                if (context != nullptr) {
                    std::cout << "Redis连接失败: " << context->errstr << std::endl;
                    redisFree(context);
                }
                continue;
            }

            // 密码认证
            auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
            if (reply->type == REDIS_REPLY_ERROR) {
                std::cout << "Redis认证失败: " << reply->str << std::endl;
                //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
                freeReplyObject(reply);
                continue;
            }

            //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
            freeReplyObject(reply);
            std::cout << "认证成功" << std::endl;

            // 显式选择 DB 0
            auto select_reply = (redisReply*)redisCommand(context, "SELECT 0");
            if (select_reply == nullptr || select_reply->type == REDIS_REPLY_ERROR) {
                std::cout << "选择数据库失败" << std::endl;
                if (select_reply) freeReplyObject(select_reply);
                redisFree(context);
                continue;
            }
            freeReplyObject(select_reply);

            connections_.push(context);
        }

    }

    ~RedisConPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!connections_.empty()) {
            auto* context = connections_.front();
            connections_.pop();
            redisFree(context);
        }
    }

    redisContext* getConnection() {
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
        auto* context = connections_.front();
        connections_.pop();
        return context;
    }

    void returnConnection(redisContext* context) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_) {
            return;
        }
        connections_.push(context);
        cond_.notify_one();
    }

    void Close() {
        b_stop_ = true;
        cond_.notify_all();
    }

private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    const char* host_;
    int port_;
    std::queue<redisContext*> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};


// RAII 类
class RedisConnGuard {
public:
    // 构造时借出连接
    RedisConnGuard(std::unique_ptr<RedisConPool>& pool) : _pool(pool) {
        _context = _pool->getConnection();
    }

    // 析构时自动归还连接
    ~RedisConnGuard() {
        if (_context && _pool) {
            _pool->returnConnection(_context);
        }
    }

    // 获取内部连接指针
    redisContext* get() { 
        return _context; 
    }

    // 禁用拷贝，确保连接归还逻辑唯一
    RedisConnGuard(const RedisConnGuard&) = delete;
    RedisConnGuard& operator=(const RedisConnGuard&) = delete;

private:
    std::unique_ptr<RedisConPool>& _pool;
    redisContext* _context;
};


RedisMgr::RedisMgr() {
    auto& gCfgMgr = ConfigMgr::getInst();
    auto host = gCfgMgr["Redis"]["Host"];
    auto port = gCfgMgr["Redis"]["Port"];
    auto pwd = gCfgMgr["Redis"]["Passwd"];
    con_pool_.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}

RedisMgr::~RedisMgr() {
    close();
}

// 字符串操作
bool RedisMgr::get(const std::string& key, std::string& value) {
    RedisConnGuard guard(con_pool_); // 使用RAII自动借出归还connection
    auto connect = guard.get();
    if (connect == nullptr) {
        std::cout << "无法获取Redis连接" << std::endl;
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());

    if (reply == nullptr) {
        std::cout << "[ GET " << key << " ] 命令执行返回空指针" << std::endl;
        return false;
    }

    // --- 修改点：区分处理 NIL (Key不存在) 情况 ---
    if (reply->type == REDIS_REPLY_NIL) {
        std::cout << "[ GET " << key << " ] 失败：Key 在 Redis 中不存在" << std::endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING) {
        std::cout << "[ GET " << key << " ] 失败：返回类型错误，类型编号: " << reply->type << std::endl;
        freeReplyObject(reply);
        return false;
    }

    value = reply->str;
    freeReplyObject(reply);
    std::cout << "成功获取验证码 [ GET " << key << " ] value: " << value << std::endl;
    return true;
}

bool RedisMgr::set(const std::string& key, const std::string& value) {
    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());

    //如果返回NULL则说明执行失败
    if (reply == nullptr)
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        return false;
    }

    //如果执行失败则释放连接
    if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
    return true;
}

// 列表操作
bool RedisMgr::lPush(const std::string& key, const std::string& value) {
    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());

    if (NULL == reply)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::lPop(const std::string& key, std::string& value) {
    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());

    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    value = reply->str;
    std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::rPush(const std::string& key, const std::string& value) {
    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());

    if (reply == nullptr) {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::rPop(const std::string& key, std::string& value) {
    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "RPOP %s ", key.c_str());

    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    value = reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

// 哈希表操作
bool RedisMgr::hSet(const std::string& key, const std::string& hkey, const std::string& value) {
    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

// 处理二进制数据的版本
bool RedisMgr::hSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen) {
    const char* argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;

    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

std::string RedisMgr::hGet(const std::string& key, const std::string& hkey) {
    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();

    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return "";
    }

    auto* reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);

    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
        return "";
    }

    std::string value = reply->str;
    freeReplyObject(reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    return value;
}

bool RedisMgr::hDel(const std::string& key, const std::string& field) {
    auto connect = con_pool_->getConnection();
    if (connect == nullptr) {
        return false;
    }

    Defer defer([&connect, this]() {
        con_pool_->returnConnection(connect);
        });

    redisReply* reply = (redisReply*)redisCommand(connect, "HDEL %s %s", key.c_str(), field.c_str());
    if (reply == nullptr) {
        std::cerr << "HDEL command failed" << std::endl;
        return false;
    }

    bool success = false;
    if (reply->type == REDIS_REPLY_INTEGER) {
        success = reply->integer > 0;
    }

    freeReplyObject(reply);
    return success;
}


// 删除操作（资源回收）
bool RedisMgr::del(const std::string& key) {
    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

// 判断键值是否存在
bool RedisMgr::existsKey(const std::string& key) {
    RedisConnGuard guard(con_pool_);
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto* reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

// 关闭
void RedisMgr::close() {
    con_pool_->Close();
}