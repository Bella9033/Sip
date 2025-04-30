// ev_thread_pool.cpp - 修复版

#include "ev_thread_pool.h"
#include "common.h"
#include <unordered_map>

ThreadPool::ThreadPool(size_t thread_count)
{
    if (thread_count == 0) throw std::invalid_argument("thread_count must be > 0");
    
    // 修复: 初始化退出标志为普通布尔值
    worker_exit_flags_.resize(thread_count, false);
    
    // 创建工作线程
    for (size_t i = 0; i < thread_count; ++i)
        addWorker();
    
    LOG(INFO) << "ThreadPool started with " << thread_count << " threads.";
}

ThreadPool::~ThreadPool()
{
    shutdown();
}

void ThreadPool::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (stop_) return;
        stop_ = true;
        
        // 修复: 设置所有工作线程的退出标志
        for (size_t i = 0; i < worker_exit_flags_.size(); ++i) {
            worker_exit_flags_[i] = true;
        }
    }
    
    // 通知所有线程
    condition_.notify_all();
    
    // 等待所有线程完成
    for (auto& w : workers_)
    {
        if (w.joinable())
            w.join();
    }
    
    LOG(INFO) << "ThreadPool shutdown complete.";
}

// 修复: 实现动态调整线程池大小
void ThreadPool::setThreadCount(size_t n) 
{
    if (n == 0) {
        LOG(ERROR) << "Thread count must be greater than 0";
        return;
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (stop_) return;
    
    size_t current = workers_.size();
    
    if (n > current) {
        // 扩大线程池
        size_t to_add = n - current;
        
        // 扩展退出标志数组
        worker_exit_flags_.resize(n, false);
        
        // 添加新工作线程
        for (size_t i = 0; i < to_add; ++i) {
            addWorker();
        }
        
        LOG(INFO) << "ThreadPool expanded from " << current << " to " << n << " threads";
    }
    else if (n < current) {
        // 减少线程池
        removeWorker(current - n);
        LOG(INFO) << "ThreadPool reduced from " << current << " to " << n << " threads";
    }
}

void ThreadPool::addWorker()
{
    size_t id = workers_.size();
    
    // 确保退出标志数组的大小足够
    if (id >= worker_exit_flags_.size()) {
        worker_exit_flags_.resize(id + 1, false);
    }
    
    // 创建并启动工作线程
    workers_.emplace_back([this, id] { 
        // 注册线程ID到索引的映射
        {
            std::lock_guard<std::mutex> lock(thread_map_mutex_);
            thread_to_index_[std::this_thread::get_id()] = id;
        }
        
        try {
            this->worker_loop();
        } catch (const std::exception& e) {
            LOG(ERROR) << "Worker thread " << id << " exited with exception: " << e.what();
        }
        
        // 线程退出后清理映射
        {
            std::lock_guard<std::mutex> lock(thread_map_mutex_);
            thread_to_index_.erase(std::this_thread::get_id());
        }
    });
}

// 修复: 添加移除工作线程的实现
void ThreadPool::removeWorker(size_t count) 
{
    // 标记要移除的线程
    size_t current = workers_.size();
    size_t to_remove = std::min(count, current);
    
    for (size_t i = current - to_remove; i < current; ++i) {
        if (i < worker_exit_flags_.size()) {
            worker_exit_flags_[i] = true;
        }
    }
    
    // 通知所有线程检查退出标志
    condition_.notify_all();
}

size_t ThreadPool::size() const 
{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return workers_.size();
}

bool ThreadPool::isRunning() const 
{
    return !stop_;
}

size_t ThreadPool::pendingTasks() const 
{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

size_t ThreadPool::activeThreads() const 
{
    return active_thread_count_;
}

void ThreadPool::worker_loop()
{
    // 获取当前线程的索引
    size_t worker_id;
    {
        std::lock_guard<std::mutex> lock(thread_map_mutex_);
        auto it = thread_to_index_.find(std::this_thread::get_id());
        if (it == thread_to_index_.end()) {
            LOG(ERROR) << "Worker thread ID not found in map";
            return;
        }
        worker_id = it->second;
    }
    
    while (true)
    {
        Task task;
        bool got_task = false;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // 修复: 获取正确的退出标志
            bool should_exit = worker_id < worker_exit_flags_.size() && 
                              worker_exit_flags_[worker_id];
            
            condition_.wait(lock, [this, should_exit, &got_task] { 
                return stop_ || should_exit || !tasks_.empty(); 
            });
            
            if ((stop_ && tasks_.empty()) || should_exit) {
                // 修复: 识别优雅退出情况
                if (should_exit) {
                    LOG(INFO) << "Worker thread " << worker_id << " exiting due to pool resizing";
                }
                return;
            }
            
            if (!tasks_.empty()) {
                task = std::move(const_cast<Task&>(tasks_.top()));
                tasks_.pop();
                got_task = true;
                ++active_thread_count_;
            }
        }
        
        if (got_task) {
            try {
                task.func();
                ++total_completed_;
            }
            catch (const std::exception& e) {
                LOG(ERROR) << "Exception in thread pool task: " << e.what();
            }
            catch (...) {
                LOG(ERROR) << "Unknown exception in thread pool task.";
            }
            
            --active_thread_count_;
        }
    }
}