// task_timer.h - 修复版
#pragma once
#include "common.h"
#include "pjsip_utils.h"
#include "ev_thread.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <memory>

// 允许使用 weak_from_this()
class TaskTimer : public std::enable_shared_from_this<TaskTimer>
{
public:
    using Task = std::function<void()>;

    TaskTimer();
    ~TaskTimer();

    void start();
    void stop();
    void addTask(const Task& task);
    void setInterval(unsigned int ms);

private:
    void timerLoop();

    std::thread timer_thread_;
    std::vector<Task> tasks_;
    bool running_;
    unsigned int interval_ms_;
    
    // 保护任务列表的互斥锁
    std::mutex task_mutex_;
    
    // 使用条件变量实现更优雅的线程停止
    std::mutex cv_mutex_;
    std::condition_variable cv_;
    
    // 修改: 将stop_flag_从静态成员改为实例成员，避免共享问题
    std::atomic<bool> stop_flag_{false};
};