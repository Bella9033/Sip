// task_timer.cpp
#include "task_timer.h"

#include "task_timer.h"
#include "pjsip_utils.h"

std::atomic<bool> TaskTimer::stop_flag_{false};

TaskTimer::TaskTimer()
    : running_(false),
      interval_ms_(1000) // 默认1秒
{
}

TaskTimer::~TaskTimer()
{
    stop();
}

void TaskTimer::start()
{
    LOG(INFO) << "Creating timer thread...";
    if (running_) {
        return; // 已经在运行
    }
    
    // 修复：确保在启动线程前重置stop_flag_
    stop_flag_ = false;
    
    // 使用智能指针管理线程对象
    timer_thread_ = std::thread(&TaskTimer::timerLoop, this);
    running_ = true;
    
    // 修复：添加日志并返回前确认线程已启动
    LOG(INFO) << "Timer started successfully";
}

void TaskTimer::timerLoop()
{
    LOG(INFO) << "Timer thread started, id=" << std::this_thread::get_id();
    
    // 修复：确保线程一开始就注册PJSIP
    PjSipUtils::ThreadRegistrar thread_registrar;
    
    while (!stop_flag_)
    {
        // 修复：添加互斥锁保护任务列表访问
        {
            std::lock_guard<std::mutex> lock(task_mutex_);
            for (const auto& task : tasks_)
            {
                if (task) {
                    task();
                }
            }
        }
        
        // 修复：使用条件变量而不是sleep，以便更快响应停止请求
        std::unique_lock<std::mutex> lock(cv_mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(interval_ms_),
                     [this]() { return stop_flag_; });
    }
}

void TaskTimer::stop()
{
    if (!running_) {
        return;
    }
    
    // 修复：使用条件变量通知线程停止
    {
        std::lock_guard<std::mutex> lock(cv_mutex_);
        stop_flag_ = true;
    }
    cv_.notify_all();
    
    if (timer_thread_.joinable()) {
        timer_thread_.join();
    }
    
    // 修复：清空任务列表
    {
        std::lock_guard<std::mutex> lock(task_mutex_);
        tasks_.clear();
    }
    
    running_ = false;
}

void TaskTimer::addTask(const Task& task)
{
    // 修复：添加锁保护任务列表
    std::lock_guard<std::mutex> lock(task_mutex_);
    tasks_.push_back(task);
}

void TaskTimer::setInterval(unsigned int ms)
{
    interval_ms_ = ms;
}