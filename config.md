# 运行环境配置

## 操作系统
- Linux (Ubuntu 20.04+ 或 Debian 11+ 推荐)

## 编译器
- g++ 11+ 或 clang++ 14+
- 支持 C++20/23 标准

## 构建工具
- CMake 3.20+

## 构建命令
```bash
mkdir build && cd build
cmake ..
make
```

## 运行命令
```bash
# 启动服务器（默认端口 8080）
./db_server [port]

# 启动客户端
./db_client [host] [port]

# 运行测试
./db_test
```

## 依赖
- POSIX sockets (Linux 内置)
- pthread (Linux 内置)
- 无第三方依赖
