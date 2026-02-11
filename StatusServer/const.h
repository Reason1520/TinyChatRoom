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
#include <functional>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

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
    TokenInvalid = 1010,    // Token无效
    UidInvalid = 1011,      // UID无效
    RPCGetFailed = 1012,    // 找不到chatServer
};

// 配置管理类
class ConfigMgr;
extern ConfigMgr gCfgMgr;

// Defer类
class Defer {
public:
    Defer(std::function<void()> func) : func_(func) {}

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
#define LOCK_PREFIX "lock_"
#define USER_SESSION_PREFIX "usession_"
#define LOCK_COUNT "lockcount"

//分布式锁的持有时间
#define LOCK_TIME_OUT 10
//分布式锁的重试时间
#define ACQUIRE_TIME_OUT 5

// 传递数据相关
#define MAX_LENGTH 1024*2
#define HEAD_TOTAL_LEN 4    // 头部总长度
#define HEAD_ID_LEN 2       // 头部id长度
#define HEAD_DATA_LEN 2     // 头部数据长度
#define MAX_RECVQUE 10000
#define MAX_SENDQUE 10000

// 消息类型
enum MSG_IDS {
    MSG_CHAT_LOGIN = 1005, //用户登陆
    MSG_CHAT_LOGIN_RSP = 1006, //用户登陆回包
    ID_SEARCH_USER_REQ = 1007, //用户搜索请求
    ID_SEARCH_USER_RSP = 1008, //搜索用户回包
    ID_ADD_FRIEND_REQ = 1009, //申请添加好友请求
    ID_ADD_FRIEND_RSP = 1010, //申请添加好友回复
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,  //通知用户添加好友申请
    ID_AUTH_FRIEND_REQ = 1013,  //认证好友请求
    ID_AUTH_FRIEND_RSP = 1014,  //认证好友回复
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //通知用户认证好友申请
    ID_TEXT_CHAT_MSG_REQ = 1017, //文本聊天信息请求
    ID_TEXT_CHAT_MSG_RSP = 1018, //文本聊天信息回复
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //通知用户文本聊天信息
    ID_NOTIFY_OFF_LINE_REQ = 1021, //通知用户下线
};

// 生成唯一的uuid
std::string generateUUID();

#endif //CONST_H