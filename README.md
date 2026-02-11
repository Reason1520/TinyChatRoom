# TinyChatRoom 🗨️

一个基于 Qt 和 C++ 构建的分布式即时聊天系统，包含完整的客户端和多服务器后端架构。

## 📋 功能特性

### 客户端功能
- 🔐 用户注册、登录、密码重置
- 📧 邮箱验证码验证
- 🔍 用户搜索（支持UID和用户名搜索）
- 👥 好友申请与管理
- 💬 一对一文本聊天
- 💾 聊天记录持久化存储与分页加载
- 🎨 现代化的聊天气泡界面
- 📱 联系人列表管理

### 服务端功能
- 🌐 网关服务器（GateServer）- 处理HTTP请求
- 💬 聊天服务器（ChatServer）- 处理TCP长连接和消息转发
- 📊 状态服务器（StatusServer）- 管理用户在线状态
- ✉️ 验证服务器（VerifyServer）- 邮件验证码服务
- 🔒 Redis 分布式锁保证消息写入一致性
- 📄 聊天消息持久化到 MySQL，支持游标分页查询

## 🏗️ 系统架构

```
                    ┌──────────────┐
                    │   TinyChat   │
                    │   (Qt客户端) │
                    └───────┬──────┘
                            │
            ┌───────────────┼───────────────┐
            │               │               │
            ▼               ▼               ▼
    ┌───────────────┐ ┌───────────┐ ┌───────────────┐
    │  GateServer   │ │ChatServer │ │ StatusServer  │
    │  (HTTP网关)   │ │(TCP长连接)│ │  (状态管理)   │
    └───────┬───────┘ └─────┬─────┘ └───────┬───────┘
            │               │               │
            │      ┌────────┴────────┐      │
            │      │                 │      │
            ▼      ▼                 ▼      ▼
        ┌───────────┐           ┌───────────┐
        │   MySQL   │           │   Redis   │
        │  (数据库) │           │  (缓存)   │
        └───────────┘           └───────────┘
                                     │
                    ┌────────────────┘
                    ▼
            ┌───────────────┐
            │ VerifyServer  │
            │  (Node.js)    │
            └───────────────┘
```

## 🛠️ 技术栈

### 客户端
- **Qt 6** - 跨平台GUI框架
- **C++17** - 编程语言
- **QSS** - 界面样式表

### 服务端
- **Boost.Asio** - 异步网络库
- **gRPC** - 服务间RPC通信
- **Protobuf** - 数据序列化
- **jsoncpp** - JSON解析
- **MySQL** - 关系型数据库
- **Redis** - 缓存和会话管理
- **Node.js** - 验证服务

## 📁 项目结构

```
TinyChatRoom/
├── TinyChat/              # Qt客户端
│   ├── res/               # 资源文件（图标、图片等）
│   ├── style/             # QSS样式表
│   ├── main.cpp           # 主入口
│   ├── mainwindow.*       # 主窗口
│   ├── logindialog.*      # 登录对话框
│   ├── signupdialog.*     # 注册对话框
│   ├── resetdialog.*      # 重置密码对话框
│   ├── chatdialog.*       # 聊天对话框
│   ├── tcpmgr.*           # TCP连接管理
│   ├── httpmgr.*          # HTTP请求管理
│   └── usermgr.*          # 用户信息管理
│
├── GateServer/            # 网关服务器
│   ├── main.cpp           # 主入口
│   ├── cserver.*          # HTTP服务器
│   ├── httpconnection.*   # HTTP连接处理
│   ├── logicsystem.*      # 业务逻辑
│   ├── mysqldao.*         # MySQL访问层
│   ├── redismgr.*         # Redis管理
│   └── config.ini         # 配置文件
│
├── ChatServer/            # 聊天服务器
│   ├── main.cpp           # 主入口
│   ├── cserver.*          # TCP服务器
│   ├── csession.*         # 会话管理
│   ├── logicsystem.*      # 消息处理逻辑
│   ├── chatserviceimpl.*  # gRPC服务实现
│   ├── mysqldao.*         # MySQL访问层
│   ├── redismgr.*         # Redis管理
│   ├── distlock.*         # Redis分布式锁
│   ├── utils.*            # 工具函数（时间戳等）
│   └── config.ini         # 配置文件
│
├── StatusServer/          # 状态服务器
│   ├── main.cpp           # 主入口
│   ├── statusserviceimpl.*# gRPC服务实现
│   └── config.ini         # 配置文件
│
└── VerifyServer/          # 验证服务器（Node.js）
    ├── server.js          # 主入口
    ├── email.js           # 邮件发送
    ├── redis.js           # Redis操作
    └── config.json        # 配置文件
```

## 🚀 快速开始

### 环境要求
- Qt 6.x
- Visual Studio 2022
- MySQL 8.x
- Redis
- Node.js 18+

### 服务端部署

1. **启动 MySQL 和 Redis 服务**

2. **配置数据库**
   - 创建数据库并导入表结构

3. **启动验证服务器**
   ```bash
   cd VerifyServer
   npm install
   node server.js
   ```

4. **启动后端服务器**
   - 按顺序启动：StatusServer → GateServer → ChatServer

### 客户端运行

1. 使用 Qt Creator 或 Visual Studio 打开 `TinyChat.slnx`
2. 配置 Qt 环境和依赖库
3. 编译运行

## 📌 更新日志

### v2 - 聊天消息持久化（2026-02-11）
- 新增聊天消息持久化存储到 MySQL
- 新增聊天线程（ChatThread）和消息（ChatMessage）数据模型
- 实现基于游标的分页查询，支持加载历史消息
- 引入 Redis 分布式锁，保证并发场景下消息写入的一致性
- 新增时间戳工具类

### v1 - 初始版本（2026-02-03）
- 完整的用户注册、登录、密码重置流程
- 好友搜索、申请、认证功能
- 一对一实时文本聊天
- 多服务器分布式架构

## 教程以及原项目地址

https://github.com/secondtonone1/llfcchat