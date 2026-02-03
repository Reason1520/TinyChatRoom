@echo off
setlocal

:: ================= 配置区域 =================
:: Proto 文件名
set PROTO_NAME=message.proto

:: protoc.exe 的路径
set PROTOC_EXE=C:\Users\lueying\Code\grpc\visualpro\third_party\protobuf\Debug\protoc.exe

:: gRPC 插件的路径
set GRPC_PLUGIN=C:\Users\lueying\Code\grpc\visualpro\Debug\grpc_cpp_plugin.exe

:: 输出目录（. 代表当前目录）
set OUT_DIR=.
:: ============================================

echo [INFO] 正在生成 Protobuf 代码...
"%PROTOC_EXE%" --cpp_out="%OUT_DIR%" "%PROTO_NAME%"
if %errorlevel% neq 0 (
    echo [ERROR] Protobuf 生成失败！
    pause
    exit /b %errorlevel%
)

echo [INFO] 正在生成 gRPC 服务代码...
"%PROTOC_EXE%" -I="." --grpc_out="%OUT_DIR%" --plugin=protoc-gen-grpc="%GRPC_PLUGIN%" "%PROTO_NAME%"
if %errorlevel% neq 0 (
    echo [ERROR] gRPC 生成失败！
    pause
    exit /b %errorlevel%
)

echo [SUCCESS] 所有代码生成成功！
pause