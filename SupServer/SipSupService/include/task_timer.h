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
    using TimerCallback = std::function<void()>;

    // 修改: 添加默认超时参数
    explicit TaskTimer(int interval_seconds = 3, 
                      std::chrono::milliseconds timeout = std::chrono::milliseconds{10000});
    ~TaskTimer();

    TaskTimer(const TaskTimer&) = delete;
    TaskTimer& operator=(const TaskTimer&) = delete;

    void startTaskTimer();
    void stop();

    void setCallback(TimerCallback cb);
    
    // 修改: 添加状态查询方法
    bool isRunning() const { return is_running_; }

private:
    void timerLoop();

    std::atomic<bool> is_running_{ false };
    std::atomic<bool> stop_flag_{ false };
    int interval_seconds_;
    
    // 修改: 添加超时配置
    std::chrono::milliseconds thread_timeout_;

    std::mutex cb_mutex_;
    TimerCallback callback_;
    void* cb_param_{ nullptr };

    std::future<void> thread_future_;
};