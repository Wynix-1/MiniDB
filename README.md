# MiniDB - 微型关系型数据库管理系统

一个对标MySQL的微型关系型数据库管理系统，采用C/S架构，使用C++23标准开发。

## 项目特点

- **纯手写容器**：不使用STL容器，自行实现Array、String、List、Map等基础容器
- **B+树索引**：采用B+树实现主键索引，加速查询操作
- **C/S架构**：服务器和客户端分离，通过TCP/IP协议通信
- **JSON序列化**：使用JSON格式进行网络数据传输
- **数据持久化**：支持数据的文件存储和加载
- **MySQL风格CLI**：模仿MySQL交互式命令行界面

## 项目结构

```
project/
├── include/                  # 头文件目录
│   ├── array.hpp            # 自定义动态数组
│   ├── string.hpp           # 自定义字符串
│   ├── list.hpp             # 自定义双向链表
│   ├── map.hpp              # 自定义映射（基于BST）
│   ├── exception.hpp        # 自定义异常类
│   ├── logger.hpp           # 日志系统
│   ├── column.hpp           # 表列定义
│   ├── row.hpp              # 表行数据
│   ├── table.hpp            # 表管理
│   ├── database.hpp         # 数据库管理
│   ├── bplus_tree.hpp       # B+树索引
│   ├── sql_parser.hpp       # SQL解析器
│   ├── storage_engine.hpp   # 存储引擎
│   ├── json_serializer.hpp  # JSON序列化
│   ├── tcp_server.hpp       # TCP服务器
│   └── tcp_client.hpp       # TCP客户端
├── src/
│   ├── server/main.cpp      # 服务器入口
│   ├── client/main.cpp      # 客户端入口
│   └── test/test_main.cpp   # 单元测试
├── CMakeLists.txt           # CMake构建配置
├── Makefile                 # Makefile构建配置
├── config.md                # 运行环境配置
└── .gitignore               # Git忽略配置
```

## 编译

### 使用Makefile（推荐）

```bash
make all          # 编译所有目标
make clean        # 清理编译产物
make test         # 编译并运行测试
make server       # 编译并运行服务器
make client       # 编译并运行客户端
```

### 使用CMake

```bash
mkdir build && cd build
cmake ..
make
```

## 运行

### 启动服务器

```bash
./db_server [port]    # 默认端口8080
```

### 启动客户端

```bash
./db_client [host] [port]    # 默认127.0.0.1:8080
```

### 运行测试

```bash
./db_test
```

## 支持的SQL语句

### DDL（数据定义语言）

```sql
CREATE DATABASE <dbname>;                          -- 创建数据库
DROP DATABASE <dbname>;                            -- 删除数据库
USE <dbname>;                                      -- 切换数据库
CREATE TABLE <name> (<col> <type> [primary], ...); -- 创建表
DROP TABLE <name>;                                 -- 删除表
```

### DML（数据操作语言）

```sql
SELECT <col> FROM <table> [WHERE <cond>];          -- 查询数据
INSERT <table> VALUES (<value>, ...);              -- 插入数据
UPDATE <table> SET <col> = <value> [WHERE <cond>]; -- 更新数据
DELETE <table> [WHERE <cond>];                     -- 删除数据
```

### 数据类型

- `int`：整数类型
- `string`：字符串类型（最长256字符，UTF-8编码）

### 比较操作符

- `=`：等于
- `<`：小于
- `>`：大于
- `<=`：小于等于
- `>=`：大于等于
- `!=`：不等于

## 客户端命令

```
help, \h         - 显示帮助信息
exit, quit, \q   - 退出客户端
clear, \c        - 清除当前输入
status, \s       - 显示服务器状态
```

## 使用示例

```
$ ./db_server
[INFO] MiniDB Server starting...
[INFO] Server is running on port 8080. Press Ctrl+C to stop.

$ ./db_client

Welcome to the MiniDB monitor. Commands end with ; or \g.
Your MiniDB connection id is 1
Server version: 1.0.0 MiniDB Server

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mini_db> create database testdb;
Query OK, database created
(0.15 sec)

mini_db> use testdb;
Database changed
(0.02 sec)

mini_db> create table person (id int primary, name string);
Query OK, table created
(0.08 sec)

mini_db> insert person values(1001, "peter");
Query OK, 1 row affected
(0.12 sec)

mini_db> insert person values(1002, "alice");
Query OK, 1 row affected
(0.10 sec)

mini_db> select * from person;
+------+------+
| id   | name |
+------+------+
| 1001 | peter|
| 1002 | alice|
+------+------+
2 rows in set
(0.05 sec)

mini_db> select name from person where id = 1001;
+-------+
| name  |
+-------+
| peter |
+-------+
1 row in set
(0.03 sec)

mini_db> exit
Bye
```

## 数据存储

数据文件存储在`data/`目录下：

```
data/
└── testdb/              # 数据库目录
    └── person.dat       # 表数据文件
```

## 技术栈

- **C++23**：现代C++标准
- **POSIX Sockets**：网络通信
- **B+树**：索引结构
- **JSON**：数据序列化
- **多线程**：并发处理

## 依赖

- g++ 11+ 或 clang++ 14+
- Linux系统
- CMake 3.20+（可选）
- 无第三方库依赖
