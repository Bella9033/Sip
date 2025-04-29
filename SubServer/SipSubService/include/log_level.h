#pragma once

#include <glog/logging.h>   // Google日志库
#include <gflags/gflags.h>  // Google命令行标志库
#include <signal.h>         // 信号处理


class SetLogLevel
{
public:
    enum class LogLevel{
        INFO = google::GLOG_INFO,      // 确保值完全匹配
        WARNING = google::GLOG_WARNING,
        ERROR = google::GLOG_ERROR
    };

    explicit SetLogLevel(LogLevel level = LogLevel::INFO);
    ~SetLogLevel();

    void configureLogging();


private:
    LogLevel loglevel_; 
    static bool is_initialized_; // 防止重复初始化
};
