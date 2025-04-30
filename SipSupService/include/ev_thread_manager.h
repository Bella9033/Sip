// ev_thread_manager.h
#pragma once

#include <thread>
#include <chrono>
#include <string>
#include <map>
#include <mutex>

// 线程优先级
enum class ThreadPriority { LOW, NORMAL, HIGH };

// 线程运行状态
enum class ThreadState { IDLE, RUNNING, COMPLETED, FAILED };

// 线程运行日志结构
struct ThreadLogs {
    std::thread::id thread_id;
    ThreadState thread_state{ThreadState::IDLE};
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::string last_error;
};

// 线程日志管理单例，支持多线程安全查询和记录
class EVThreadManager {
    public:
        static EVThreadManager& instance();
        void addThreadLog(const ThreadLogs& log);
        bool getThreadLog(std::thread::id tid, ThreadLogs& out_log);
    private:
        std::mutex mutex_;
        std::map<std::thread::id, ThreadLogs> threads_;
};