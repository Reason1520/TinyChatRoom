#ifndef CONST_H
#define CONST_H

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

/******************************************************************************
 * @file       const.h
 * @brief      存放一些用到的常量定义和一些url解析函数
 *
 * @author     lueying
 * @date       2025/12/11
 * @history
 *****************************************************************************/

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//char 转为16进制
unsigned char to_hex(unsigned char x);

// 16进制转十进制
unsigned char from_hex(unsigned char x);

// url编码
std::string url_encode(const std::string& str);

// url解码
std::string url_decode(const std::string& str);

// 错误枚举类
enum ErrorCodes {
    Success = 0,
    Error_Json = 1001,      // Json解析错误
    RPCFailed = 1002,       // RPC请求错误
    VerifyExpired = 1003,   // 验证码过期
    VerifyCodeErr = 1004,   // 验证码错误
    UserExist = 1005,       // 用户已存在
    PwdConfirmErr = 1006,   // 密码确认错误
    EmailNotMatch = 1007,   // 用户名和邮箱不匹配
    PwdUpdateFailed = 1008, // 密码更新失败
    PasswdInvalid = 1009,   // 用户密码不正确
    TokenInvalid = 1010,    //Token失效
    UidInvalid = 1011,      //uid无效
    RPCGetFailed = 1012,    // 找不到chatServer
};

// 用户信息类
struct UserInfo {
    int uid;              // 用户唯一UID
    std::string name;     // 用户名
    std::string email;    // 绑邮箱
    std::string pwd;      // 密码
};

// 配置管理类
class ConfigMgr;
extern ConfigMgr gCfgMgr;

// Defer类
class Defer {
public:
    // 接受一个lambda表达式或者函数指针
    Defer(std::function<void()> func) : func_(func) {}

    // 析构函数中执行传入的函数
    ~Defer() {
        func_();
    }

private:
    std::function<void()> func_;
};

// redis前缀
#define CODEPREFIX "code_"
#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"
#define NAME_INFO  "nameinfo_"

#endif //CONST_H