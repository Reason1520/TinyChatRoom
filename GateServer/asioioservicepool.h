#ifndef GATE_SERVER_ASIOIOSERVICEPOOL_H
#define GATE_SERVER_ASIOIOSERVICEPOOL_H

#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"

/******************************************************************************
 * @file       asioioservicepool.h
 * @brief      ASIO IOContext Pool 结构，让多个iocontext跑在不同的线程中
 *
 * @author     lueying
 * @date       2025/12/19
 * @history
 *****************************************************************************/

class AsioIOServicePool :public Singleton<AsioIOServicePool>
{
    friend Singleton<AsioIOServicePool>;
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work;
    using WorkPtr = std::unique_ptr<Work>;
    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
    // 使用 round-robin 的方式返回一个 io_service
    boost::asio::io_context& getIOService();
    void stop();
private:
    AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);
    std::vector<IOService> ioServices_;
    std::vector<WorkPtr> works_;
    std::vector<std::thread> threads_;
    std::size_t nextIOService_;
};

#endif // GATE_SERVER_ASIOIOSERVICEPOOL_H