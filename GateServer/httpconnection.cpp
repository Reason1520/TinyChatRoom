#include "httpconnection.h"
#include <iostream>
#include "logicsystem.h"

/******************************************************************************
 * @file       httpconnection.h
 * @brief      http链接类
 *
 * @author     lueying
 * @date       2025/12/11
 * @history
 *****************************************************************************/

HttpConnection::HttpConnection(tcp::socket socket) : socket_(std::move(socket)) 
{

}

HttpConnection::HttpConnection(net::io_context& io_context) : socket_(io_context) {

}

// 获取socket
tcp::socket& HttpConnection::getSocket() {
	return socket_;
}

// 开始处理连接
void HttpConnection::start() {
	auto self = shared_from_this();
	// 异步地读取完整的 HTTP 消息
	http::async_read(socket_, buffer_, request_, [self](beast::error_code ec, std::size_t bytes_transferred) {
		try {
			if (ec) {
				std::cout << "http read error: " << ec.message() << std::endl;
				return;
			}

			// 处理读到的数据
			// 这个参数暂时不用
			boost::ignore_unused(bytes_transferred);
			self->handleRequest();
			self->checkDeadline();
		}
		catch (const std::exception& e) {
			std::cerr << "Exception: " << e.what() << std::endl;
		}
	});
}

// 处理请求
void HttpConnection::handleRequest() {
	// 设置版本
	response_.version(request_.version());
	// 设置为短链接（网关不需要持久连接）
	response_.keep_alive(false);
	// 处理GET请求
	if (request_.method() == http::verb::get) {
		preParseGetParam();
		// 调用LogicSystem处理GET请求
		bool success = LogicSystem::getInstance()->handleGet(get_url_, shared_from_this());
		if (!success) {
			response_.result(http::status::not_found);					// 设置 404 状态码
			response_.set(http::field::content_type, "text/plain");
			beast::ostream(response_.body()) << "url not found\r\n";	// 写入简单的错误体
			writeResponse();											// 立即发送响应
			return;
		}
		// 处理成功
		response_.result(http::status::ok);					// 设置 200 状态码
		response_.set(http::field::server, "GateServer");	// 设置自定义的server头部
		writeResponse();									// 立即发送响应
		return;
	}
	// 处理POST请求
	if (request_.method() == http::verb::post) {
		bool success = LogicSystem::getInstance()->handlePost(request_.target(), shared_from_this());
		if (!success) {
			response_.result(http::status::not_found);
			response_.set(http::field::content_type, "text/plain");
			beast::ostream(response_.body()) << "url not found\r\n";
			writeResponse();
			return;
		}

		response_.result(http::status::ok);
		response_.set(http::field::server, "GateServer");
		writeResponse();
		return;
	}
}

// 写入消息
void HttpConnection::writeResponse() {
	auto self = shared_from_this();
	response_.content_length(response_.body().size());
	http::async_write(
		socket_,
		response_,
		[self](beast::error_code ec, std::size_t)
		{
			// 断开发送端
			self->socket_.shutdown(tcp::socket::shutdown_send, ec);
			self->deadlineTimer_.cancel();
		});
}

// 检查超时
void HttpConnection::checkDeadline() {
	auto self = shared_from_this();

	deadlineTimer_.async_wait(
		[self](beast::error_code ec)
		{
			if (!ec)
			{
				// 如果超时，直接关闭socket连接
				self->socket_.close(ec);
			}
		});
}

// 参数解析函数
void HttpConnection::preParseGetParam() {
	// 提取 URI  
	auto uri = request_.target();
	// 查找查询字符串的开始位置（即 '?' 的位置）  
	auto query_pos = uri.find('?');
	if (query_pos == std::string::npos) {
		get_url_ = uri;
		return;
	}

	get_url_ = uri.substr(0, query_pos);
	std::string query_string = uri.substr(query_pos + 1);
	std::string key;
	std::string value;
	size_t pos = 0;
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto pair = query_string.substr(0, pos);
		size_t eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = url_decode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
			value = url_decode(pair.substr(eq_pos + 1));
			get_params_[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// 处理最后一个参数对（如果没有 & 分隔符）  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = url_decode(query_string.substr(0, eq_pos));
			value = url_decode(query_string.substr(eq_pos + 1));
			get_params_[key] = value;
		}
	}
}