// task_timer.h

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "common.h"

class TaskTimer {
public:
    using Task = std::function<void()>;
    
    TaskTimer();
    ~TaskTimer();
    
    void start();
    void stop();
    void addTask(const Task& task);
    void setInterval(unsigned int ms);
    
    // 修复：老接口重命名为更清晰的新接口
    void startTaskTimer() { start(); }
    
private:
    void timerLoop();
    
    std::thread timer_thread_;
    std::vector<Task> tasks_;
    bool running_;
    unsigned int interval_ms_;
    
    // 修复：添加互斥锁保护任务列表
    std::mutex task_mutex_;
    
    // 修复：使用条件变量实现更优雅的线程停止
    std::mutex cv_mutex_;
    std::condition_variable cv_;
    
    // 使用静态原子布尔值作为停止标志
    static std::atomic<bool> stop_flag_;
};