#include "cserver.h"
#include "httpconnection.h"
#include "asioioservicepool.h"
#include <iostream>

/******************************************************************************
 * @file       cserver.h
 * @brief      网关http服务器类实现
 *
 * @author     lueying
 * @date       2025/12/11
 * @history
 *****************************************************************************/

CServer::CServer(net::io_context& io_context, unsigned short port) :
	acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {

}

// 接受http函数
void CServer::start() {
    auto self = shared_from_this();
    // 通过IOServicePool连接池中获取连接
    auto& io_context = AsioIOServicePool::getInstance()->getIOService();
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
    acceptor_.async_accept(new_con->getSocket(), [self, new_con](beast::error_code ec) {
        try {
            //出错则放弃这个连接，继续监听新链接
            if (ec) {
                self->start();
                return;
            }

            //处理新链接，创建HpptConnection类管理新连接
            new_con->start();
            //继续监听
            self->start();
        }
        catch (std::exception& exp) {
            std::cout << "exception is " << exp.what() << std::endl;
            self->start();
        }
    });
}