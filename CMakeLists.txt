cmake_minimum_required(VERSION 3.20)
project(mini_db VERSION 1.0.0 LANGUAGES CXX)

# C++23标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找线程库
find_package(Threads REQUIRED)

# 服务器可执行文件
add_executable(db_server
    src/server/main.cpp
)

target_link_libraries(db_server PRIVATE Threads::Threads)

# 客户端可执行文件
add_executable(db_client
    src/client/main.cpp
)

target_link_libraries(db_client PRIVATE Threads::Threads)

# 测试可执行文件
add_executable(db_test
    src/test/test_main.cpp
)

target_link_libraries(db_test PRIVATE Threads::Threads)

# 安装规则
install(TARGETS db_server db_client DESTINATION bin)
