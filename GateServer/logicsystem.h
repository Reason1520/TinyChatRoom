#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

#include "singleton.h"
#include <functional>
#include <map>
#include "const.h"

/******************************************************************************
 * @file       logicsystem.h
 * @brief      处理http请求的逻辑系统类
 *
 * @author     lueying
 * @date       2025/12/14
 * @history
 *****************************************************************************/

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();	
    bool handleGet(std::string, std::shared_ptr<HttpConnection>);  // get请求处理函数
    bool handlePost(std::string, std::shared_ptr<HttpConnection>); // post请求处理函数
    void regGet(std::string, HttpHandler handler);                 // 注册get请求的回调函数
    void regPost(std::string, HttpHandler handler);                // 注册post请求的回调函数
private:
    LogicSystem();
    // 请求的回调函数map，key为请求的url，value为回调函数
    std::map<std::string, HttpHandler> get_handlers_;
    std::map<std::string, HttpHandler> post_handlers_;
};

#endif // LOGICSYSTEM_H