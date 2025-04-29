// task_timer.h

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

    TaskTimer(int interval_seconds = 3);
    ~TaskTimer();

    TaskTimer(const TaskTimer&) = delete;
    TaskTimer& operator=(const TaskTimer&) = delete;

    void startTaskTimer();
    void stop();

    void setCallback(TimerCallback cb);

private:
    void timerLoop();

    std::atomic<bool> is_running_{ false };
    std::atomic<bool> stop_flag_{ false };
    int interval_seconds_;

    std::mutex cb_mutex_;
    TimerCallback callback_;
    void* cb_param_{ nullptr };

    std::future<void> thread_future_;
};