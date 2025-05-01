// task_timer.h
#pragma once
#include "common.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include <future>

class TaskTimer : public std::enable_shared_from_this<TaskTimer> {
public:
    using Task = std::function<void()>;
    
    TaskTimer();
    ~TaskTimer();
    
    // 禁用拷贝和移动
    TaskTimer(const TaskTimer&) = delete;
    TaskTimer& operator=(const TaskTimer&) = delete;
    TaskTimer(TaskTimer&&) = delete;
    TaskTimer& operator=(TaskTimer&&) = delete;
    
    // 启动定时器线程
    bool start();
    // 安全停止定时器线程
    void stop();
    
    // 添加任务
    void addTask(Task task);
    // 设置时间间隔
    void setInterval(unsigned int ms);
    
    // 获取当前状态
    bool isRunning() const { return running_; }

private:
    // 定时器线程主循环
    void timerLoop();
    
    std::vector<Task> tasks_;
    std::mutex task_mutex_;          // 保护任务列表
    std::mutex thread_mutex_;        // 保护线程状态
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_requested_{false};
    unsigned int interval_ms_{3000}; // 默认3秒
    
    // 线程ID存储
    std::shared_ptr<std::atomic<std::thread::id>> timer_thread_id_;
    
    // 保存future以便管理线程生命周期
    std::shared_ptr<std::future<void>> timer_future_;
};