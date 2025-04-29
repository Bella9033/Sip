// task_timer.cpp
#include "task_timer.h"



// TaskTimer实现：定时后台线程循环，周期性调用回调
TaskTimer::TaskTimer(int interval_seconds)
    : interval_seconds_(interval_seconds) {}

TaskTimer::~TaskTimer()
{
    stop();
}

// 创建一个定时器，定期执行某些任务（比如注册更新、状态检查等）
void TaskTimer::startTaskTimer()
{
    LOG(INFO) << "create timer thread...";
    std::lock_guard<std::mutex> lock(cb_mutex_);
    // weak_from_this() 从当前对象创建一个弱引用，
    // 弱引用不会增加引用计数，防止循环引用导致内存泄漏
    // 需要确保 TaskTimer 继承自 std::enable_shared_from_this
    // 通过弱引用检查对象是否存在，如果对象存在则执行 timerLoop()
    auto self_weak = weak_from_this();

    try {
        auto worker = [this, self_weak]() 
        {
            // 确保线程注册
            PjSipUtils::ThreadRegistrar registrar;
            // self_weak.lock()：尝试获取弱引用指向的对象
            if (auto self = self_weak.lock()) 
            {
                // 调用定时器的主循环函数
                self->timerLoop();
            }
            LOG(INFO) << "Timer thread exiting...";
            // 如果主对象析构，则不会再执行timerLoop
        };
        // 修改：延长超时时间，避免过早分离线程
        thread_future_ = EVThread::createThread(
            std::move(worker),
            std::tuple<>(),
            nullptr,
            ThreadPriority::NORMAL,
            std::chrono::milliseconds{5000} // 增加到5秒
        );
        if(!thread_future_ .valid())
        {
            LOG(ERROR) << "Failed to create timer thread!";
            return;
        }
        // 标记定时器正在运行
        is_running_ = true;
        stop_flag_ = false;
        LOG(INFO) << "Timer started successfully";
    } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to start timer: " << e.what();
        is_running_ = false;
        stop_flag_ = true;
        throw;
    }
}

void TaskTimer::stop()
{
    std::lock_guard<std::mutex> lock(cb_mutex_);
    if (!is_running_) { return; } // 如果已经停止，则不再执行
  
    try {
        stop_flag_ = true;
        if (thread_future_.valid()) 
        {
            auto status = thread_future_.wait_for(std::chrono::seconds(5));
            if (status == std::future_status::timeout) 
            {
                LOG(WARNING) << "Timer thread force stop after timeout";
            }
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "Error stopping timer: " << e.what();
    }
    LOG(INFO) << "TaskTimer stopped.";
}

void TaskTimer::setCallback(TimerCallback cb)
{
    std::lock_guard<std::mutex> lock(cb_mutex_);
    callback_ = std::move(cb);
}

// 定时器线程主循环
void TaskTimer::timerLoop()
{
    PjSipUtils::ThreadRegistrar registrar; // 确保线程注册
    auto last_time = std::chrono::steady_clock::now();
    while (!stop_flag_)
    {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();

        if (elapsed >= interval_seconds_ * 1000)
        {
            last_time = current_time;
            TimerCallback cb_copy;
            {
                std::lock_guard<std::mutex> lock(cb_mutex_);
                cb_copy = callback_;
            }
            if (cb_copy)
            {
                try {
                    LOG(INFO) << "TaskTimer: invoking registration callback.";
                    cb_copy();
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Timer callback exception: " << e.what();
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}