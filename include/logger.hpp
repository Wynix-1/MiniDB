#pragma once

#include "string.hpp"
#include <iostream>
#include <fstream>

namespace mini_db {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void set_level(LogLevel level) {
        level_ = level;
    }

    void set_output_file(const String& filename) {
        if (file_.is_open()) {
            file_.close();
        }
        file_.open(filename.c_str(), std::ios::app);
        use_file_ = file_.is_open();
    }

    void log(LogLevel level, const String& message) {
        if (level < level_) return;

        String level_str;
        switch (level) {
            case LogLevel::DEBUG: level_str = "DEBUG"; break;
            case LogLevel::INFO: level_str = "INFO"; break;
            case LogLevel::WARNING: level_str = "WARNING"; break;
            case LogLevel::ERROR: level_str = "ERROR"; break;
        }

        String log_msg = "[" + level_str + "] " + message;

        if (use_file_ && file_.is_open()) {
            file_ << log_msg.c_str() << std::endl;
        } else {
            std::cout << log_msg.c_str() << std::endl;
        }
    }

    void debug(const String& message) { log(LogLevel::DEBUG, message); }
    void info(const String& message) { log(LogLevel::INFO, message); }
    void warning(const String& message) { log(LogLevel::WARNING, message); }
    void error(const String& message) { log(LogLevel::ERROR, message); }

private:
    Logger() : level_(LogLevel::DEBUG), use_file_(false) {}
    ~Logger() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel level_;
    std::ofstream file_;
    bool use_file_;
};

#define LOG_DEBUG(msg) mini_db::Logger::instance().debug(msg)
#define LOG_INFO(msg) mini_db::Logger::instance().info(msg)
#define LOG_WARNING(msg) mini_db::Logger::instance().warning(msg)
#define LOG_ERROR(msg) mini_db::Logger::instance().error(msg)

} // namespace mini_db
