#ifndef DISTLOCK_H
#define DISTLOCK_H

#include <string>
#include <hiredis.h>

/******************************************************************************
 * @file       distlock.h
 * @brief      redis分布式锁类
 *
 * @author     lueying
 * @date       2026/2/3
 * @history
 *****************************************************************************/

class DistLock
{
public:
    static DistLock& getInst();
    ~DistLock();
    std::string acquireLock(redisContext* context, const std::string& lockName,
        int lockTimeout, int acquireTimeout);

    bool releaseLock(redisContext* context, const std::string& lockName,
        const std::string& identifier);
private:
    DistLock() = default;
};

#endif // DISTLOCK_H