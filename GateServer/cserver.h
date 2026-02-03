#ifndef CSERVER_H
#define CSERVER_H

#include "const.h"

/******************************************************************************
 * @file       cserver.h
 * @brief      网关的http服务器类
 *
 * @author     lueying
 * @date       2025/12/11
 * @history
 *****************************************************************************/

class CServer : public std::enable_shared_from_this<CServer>
{
public:
	// 构造函数传一个io管理器和端口
	CServer(net::io_context& io_context, unsigned short port);
	// 没有new不用写析构函数
	// 接受http函数
	void start();

private:
	tcp::acceptor acceptor_;	// 连接接收器
};

#endif // CSERVER_H