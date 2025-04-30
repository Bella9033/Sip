// task_timer.cpp - 修改版
#include "task_timer.h"

// 修改: 添加超时参数到构造函数
TaskTimer::TaskTimer(int interval_seconds, std::chrono::milliseconds timeout)
    : interval_seconds_(interval_seconds)
    , thread_timeout_(timeout) {}

TaskTimer::~TaskTimer()
{
    stop();
}

// 创建一个定时器，定期执行某些任务（比如注册更新、状态检查等）
void TaskTimer::startTaskTimer()
{
    LOG(INFO) << "Creating timer thread...";
    
    // 修改: 确保只启动一次
    {
        std::lock_guard<std::mutex> lock(cb_mutex_);
        if (is_running_) {
            LOG(WARNING) << "Timer already running, not starting again";
            return;
        }
    }
    
    // weak_from_this() 从当前对象创建一个弱引用，
    // 弱引用不会增加引用计数，防止循环引用导致内存泄漏
    auto self_weak = weak_from_this();

    try {
        auto worker = [this, self_weak]() 
        {
            // 确保线程注册
            PjSipUtils::ThreadRegistrar registrar;
            
            // 修改: 增加线程安全检查
            LOG(INFO) << "Timer thread started, id=" << std::this_thread::get_id();
            
            // self_weak.lock()：尝试获取弱引用指向的对象
            if (auto self = self_weak.lock()) 
            {
                // 调用定时器的主循环函数
                self->timerLoop();
            }
            else {
                LOG(WARNING) << "Timer object no longer exists, exiting thread";
            }
            
            LOG(INFO) << "Timer thread exiting...";
            // 如果主对象析构，则不会再执行timerLoop
        };
        
        // 修改: 使用成员变量中的超时配置
        thread_future_ = EVThread::createThread(
            std::move(worker),
            std::tuple<>(),
            nullptr,
            ThreadPriority::NORMAL,
            thread_timeout_
        );
        
        if(!thread_future_.valid())
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
    // 修改: 先检查是否在运行中
    if (!is_running_) { 
        LOG(INFO) << "Timer not running, nothing to stop";
        return; 
    }
    
    LOG(INFO) << "Stopping timer...";
    std::lock_guard<std::mutex> lock(cb_mutex_);
  
    try {
        // 设置停止标志
        stop_flag_ = true;
        
        // 修改: 只在future有效时才等待
        if (thread_future_.valid()) 
        {
            auto status = thread_future_.wait_for(std::chrono::seconds(5));
            if (status == std::future_status::timeout) 
            {
                LOG(WARNING) << "Timer thread force stop after timeout";
            }
            else {
                LOG(INFO) << "Timer thread stopped gracefully";
            }
        }
        
        // 不管成功与否，都设置为停止状态
        is_running_ = false;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Error stopping timer: " << e.what();
        // 确保即使出现异常也能将状态重置
        is_running_ = false;
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
    
    // 修改: 使用更精确的时间控制
    auto last_time = std::chrono::steady_clock::now();
    const auto interval_ms = std::chrono::milliseconds(interval_seconds_ * 1000);
    
    while (!stop_flag_)
    {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = current_time - last_time;

        if (elapsed >= interval_ms)
        {
            last_time = current_time;
            
            // 修改: 安全地复制回调以减少锁持有时间
            TimerCallback cb_copy;
            {
                std::lock_guard<std::mutex> lock(cb_mutex_);
                cb_copy = callback_;
            }
            
            if (cb_copy)
            {
                try {
                    LOG(INFO) << "TaskTimer: invoking callback";
                    cb_copy();
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Timer callback exception: " << e.what();
                }
            }
        } else {
            // 修改: 使用更短的睡眠时间，提高对stop_flag的响应性
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    LOG(INFO) << "Timer loop exited";
}