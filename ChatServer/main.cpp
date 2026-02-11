#include <csignal>
#include <thread>
#include <mutex>
#include "asioioservicepool.h"
#include "cserver.h"
#include "configmgr.h"
#include "redismgr.h"
#include "chatserviceimpl.h"
#include "logicSystem.h"

bool bstop = false;
// 管理退出
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main() {
	auto& cfg = ConfigMgr::getInst();
	auto server_name = cfg["SelfServer"]["Name"];
	try {
		auto pool = AsioIOServicePool::getInstance();
		//将登录数设置为0
		RedisMgr::getInstance()->initCount(server_name);
		Defer derfer([server_name]() {
			RedisMgr::getInstance()->hDel(LOGIN_COUNT, server_name);
			RedisMgr::getInstance()->close();
			});

		boost::asio::io_context  io_context;
		auto port_str = cfg["SelfServer"]["Port"];
		//创建Cserver智能指针
		auto pointer_server = std::make_shared<CServer>(io_context, atoi(port_str.c_str()));
		//定义一个GrpcServer
		std::string server_address(cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"]);
		ChatServiceImpl service;
		grpc::ServerBuilder builder;
		// 监听端口和添加服务
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		service.RegisterServer(pointer_server);
		// 构建并启动gRPC服务器
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "RPC Server listening on " << server_address << std::endl;
		
		//单独启动一个线程处理grpc服务
		std::thread  grpc_server_thread([&server]() {
			server->Wait();
		});


		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool, &server](auto, auto) {
			io_context.stop();
			pool->stop();
			server->Shutdown();
		});


		//将Cserver注册给逻辑类方便以后清除连接
		LogicSystem::getInstance()->setServer(pointer_server);
		io_context.run();

		grpc_server_thread.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}

}