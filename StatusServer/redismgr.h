#ifndef REDISMGR_H
#define REDISMGR_H

#include "singleton.h"
#include <hiredis.h>

/******************************************************************************
 * @file       redismgr.h
 * @brief      redis操作封装
 *
 * @author     lueying
 * @date       2025/12/29
 * @history
 *****************************************************************************/

class RedisConPool;

class RedisMgr : public Singleton<RedisMgr>,
    public std::enable_shared_from_this<RedisMgr>
{
    friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    // 字符串操作
    bool get(const std::string& key, std::string& value);
    bool set(const std::string& key, const std::string& value);
    // 列表操作
    bool lPush(const std::string& key, const std::string& value);
    bool lPop(const std::string& key, std::string& value);
    bool rPush(const std::string& key, const std::string& value);
    bool rPop(const std::string& key, std::string& value);
    // 哈希表操作
    bool hSet(const std::string& key, const std::string& hkey, const std::string& value);
    bool hSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string hGet(const std::string& key, const std::string& hkey);
    bool hDel(const std::string& key, const std::string& field);
    // 删除操作（资源回收）
    bool del(const std::string& key);
    // 判断键值是否存在
    bool existsKey(const std::string& key);
    // 关闭
    void close();

    // 获取分布式锁
    std::string acquireLock(const std::string& lockName, int lockTimeout, int acquireTimeout);
    // 解锁
    bool releaseLock(const std::string& lockName, const std::string& identifier);
private:
    RedisMgr();

    // 连接池
    std::unique_ptr<RedisConPool> con_pool_;
};

#endif // REDISMGR_H