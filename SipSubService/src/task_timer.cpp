// task_timer.cpp - 使用EVThread修改版
#include "task_timer.h"
#include "pjsip_utils.h"
#include "ev_thread.h" // 引入EVThread
#include <chrono>

TaskTimer::TaskTimer()
    : timer_thread_id_(std::make_shared<std::atomic<std::thread::id>>()), 
      timer_future_(std::make_shared<std::future<void>>())
{
}

TaskTimer::~TaskTimer() {
    stop();
}

bool TaskTimer::start() {
    // 使用互斥锁保护线程创建
    std::lock_guard<std::mutex> lock(thread_mutex_);
    
    // 检查线程是否已在运行
    if (running_) {
        LOG(INFO) << "TaskTimer already running";
        return true;
    }
    
    // 重置停止标志
    stop_requested_ = false;
    
    try {
        // 使用EVThread创建定时器线程
        *timer_future_ = EVThread::createThread(
            &TaskTimer::timerLoop,
            std::make_tuple(this),
            timer_thread_id_,
            ThreadPriority::NORMAL
        );
        
        running_ = true;
        LOG(INFO) << "TaskTimer started successfully with thread ID: " << timer_thread_id_->load();
        return true;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to start TaskTimer: " << e.what();
        return false;
    }
}

void TaskTimer::stop() 
{
    LOG(INFO) << "Stopping TaskTimer...";
    
    // 只有在运行时才需要停止
    if (!running_)
    {
        LOG(INFO) << "TaskTimer not running, no need to stop";
        return;
    }
    
    // 发出停止信号
    stop_requested_ = true;
    cv_.notify_all();
    
    // 等待线程结束或超时
    try {
        if (timer_future_ && timer_future_->valid()) {
            // 等待最多2秒
            std::chrono::milliseconds timeout(2000);
            auto status = timer_future_->wait_for(timeout);
            
            if (status == std::future_status::timeout) {
                LOG(WARNING) << "TaskTimer thread did not exit within timeout";
            } else {
                LOG(INFO) << "TaskTimer thread exited normally";
            }
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "Exception when waiting for timer thread: " << e.what();
    }
    
    // 清空任务队列
    {
        std::lock_guard<std::mutex> lock(task_mutex_);
        tasks_.clear();
        LOG(INFO) << "Task queue cleared";
    }
    
    // 标记为未运行
    running_ = false;
    LOG(INFO) << "TaskTimer stopped successfully";
}

void TaskTimer::timerLoop() 
{
    LOG(INFO) << "Timer thread started, id=" << std::this_thread::get_id();
    
    // 确保线程一开始就注册PJSIP
    PjSipUtils::ThreadRegistrar thread_registrar;
    
    // 主定时器循环
    while (!stop_requested_) {
        // 执行所有任务
        std::vector<Task> current_tasks;
        {
            // 复制任务列表以避免长时间持有锁
            std::lock_guard<std::mutex> lock(task_mutex_);
            current_tasks = tasks_;
        }
        
        // 执行任务
        for (const auto& task : current_tasks) {
            if (task && !stop_requested_) 
            {
                try {
                    task();
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Exception in timer task: " << e.what();
                }
            }
        }
        
        // 使用条件变量等待，可快速响应停止请求
        {
            std::unique_lock<std::mutex> lock(task_mutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(interval_ms_),
                         [this]() { return stop_requested_.load(); });
        }
    }
    
    LOG(INFO) << "Timer thread exiting";
}

void TaskTimer::addTask(Task task) 
{
    if (!task) {
        LOG(WARNING) << "Attempted to add empty task to TaskTimer";
        return;
    }
    
    std::lock_guard<std::mutex> lock(task_mutex_);
    tasks_.push_back(std::move(task));
    LOG(INFO) << "Task added to TaskTimer";
}

void TaskTimer::setInterval(unsigned int ms) 
{
    interval_ms_ = ms;
    LOG(INFO) << "TaskTimer interval set to " << ms << " ms";
}