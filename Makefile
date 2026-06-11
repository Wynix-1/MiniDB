# MiniDB Makefile
# 如果CMake不可用，可以使用此Makefile直接编译

CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -O2
LDFLAGS = -pthread

INCLUDES = -I./include

# 源文件
SERVER_SRC = src/server/main.cpp
CLIENT_SRC = src/client/main.cpp
TEST_SRC = src/test/test_main.cpp

# 目标文件
SERVER_BIN = db_server
CLIENT_BIN = db_client
TEST_BIN = db_test

# 默认目标
all: $(SERVER_BIN) $(CLIENT_BIN) $(TEST_BIN)

# 服务器
$(SERVER_BIN): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "Built $@"

# 客户端
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "Built $@"

# 测试
$(TEST_BIN): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo "Built $@"

# 清理
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) $(TEST_BIN)
	@echo "Cleaned"

# 运行测试
test: $(TEST_BIN)
	./$(TEST_BIN)

# 运行服务器
server: $(SERVER_BIN)
	./$(SERVER_BIN)

# 运行客户端
client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

.PHONY: all clean test server client
