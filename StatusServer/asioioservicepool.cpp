#include "asioioservicepool.h"
#include <iostream>

using namespace std;

AsioIOServicePool::AsioIOServicePool(std::size_t size) :ioServices_(size),
works_(size), nextIOService_(0) {
    for (std::size_t i = 0; i < size; ++i) {
        works_[i] = std::unique_ptr<Work>(new Work(ioServices_[i]));
    }

    //遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
    for (std::size_t i = 0; i < ioServices_.size(); ++i) {
        threads_.emplace_back([this, i]() {
            ioServices_[i].run();   // 每个线程认领一个 io_context 并开始运行
            });
    }
}

AsioIOServicePool::~AsioIOServicePool() {
    stop();
    std::cout << "AsioIOServicePool destruct" << endl;
}

// 使用 round-robin 的方式返回一个 io_service
boost::asio::io_context& AsioIOServicePool::getIOService() {
    auto& service = ioServices_[nextIOService_++];
    if (nextIOService_ == ioServices_.size()) {
        nextIOService_ = 0;
    }
    return service;
}

void AsioIOServicePool::stop() {
    //因为仅仅执行work.reset并不能让iocontext从run的状态中退出
    //当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
    for (auto& work : works_) {
        //把服务先停止
        work->get_io_context().stop();
        work.reset();
    }

    // 等待子线程退出后才销毁
    for (auto& t : threads_) {
        t.join();
    }
}