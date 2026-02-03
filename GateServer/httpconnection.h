#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "const.h"

/******************************************************************************
 * @file       httpconnection.h
 * @brief      http链接类
 *
 * @author     lueying
 * @date       2025/12/11
 * @history
 *****************************************************************************/

class HttpConnection : public  std::enable_shared_from_this<HttpConnection> {
	friend class LogicSystem;
public:
	HttpConnection(tcp::socket socket);
	HttpConnection(net::io_context& io_context);
	void start();	// 开始处理连接
	tcp::socket& getSocket();	// 获取套接字

private:
	tcp::socket socket_;								// 套接字(看后面要不要改成beast::tcp_stream)
	beast::flat_buffer buffer_{8192};					// 缓冲区
	http::request<http::dynamic_body> request_;			// 请求消息
	http::response<http::dynamic_body> response_;		// 响应消息
	net::steady_timer deadlineTimer_{
	socket_.get_executor(), std::chrono::seconds(60) };	// 超时定时器

	std::string get_url_;										// 存储从 URI 中分离出来的 纯路径部分
	std::unordered_map<std::string, std::string> get_params_;	// 存储解析后的get参数键值对

	void checkDeadline();	// 检查计时器是否已到期
	void writeResponse();	// 写入消息
	void handleRequest();	// 处理请求
	void preParseGetParam();	// 参数解析函数
};

#endif // HTTPCONNECTION_H