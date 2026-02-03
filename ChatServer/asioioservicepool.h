#include <vector>
#include <boost/asio.hpp>
#include "singleton.h"

/******************************************************************************
 * @file       asioioservicepool.h
 * @brief      多线程IO池类
 *
 * @author     lueying
 * @date       2026/1/4
 * @history
 *****************************************************************************/

class AsioIOServicePool :public Singleton<AsioIOServicePool> {
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
    AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());
    std::vector<IOService> ioServices_;     // iocontext的集合
    std::vector<WorkPtr> works_;            // 默认工作
    std::vector<std::thread> threads_;
    std::size_t nextIOService_;
};