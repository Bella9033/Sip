// ev_thread_pool.h - 修复版

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <future>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <stdexcept>
#include <cstdint>

#include "common.h"

// ===== 任务结构体（带优先级）=====
struct ThreadTask
{
    int priority{0}; // 越大优先级越高
    std::function<void()> func;

    // 支持优先队列比较
    bool operator<(const ThreadTask& other) const
    {
        // priority越大越优先，所以小于号反向
        return priority < other.priority;
    }
};

// ===== 线程池 =====
class ThreadPool
{
public:
    using Task = ThreadTask;

    explicit ThreadPool(size_t thread_count = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    // C++11/14/17: 提交带优先级任务（返回future）
    template<class F, class... Args>
    auto submit(int priority, F&& func, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;
        auto task_ptr = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(func), std::forward<Args>(args)...));
        std::future<return_type> res = task_ptr->get_future();

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_)
            {
                LOG(ERROR) << "ThreadPool is stopped! Can't submit new task.";
                throw std::runtime_error("ThreadPool stopped");
            }
            tasks_.emplace(Task{priority, [task_ptr]() { (*task_ptr)(); }});
            ++total_enqueued_;
        }
        condition_.notify_one();
        return res;
    }

    // 修改: 实现setThreadCount方法
    void setThreadCount(size_t n); 
    size_t size() const;
    bool isRunning() const;

    // 统计信息
    uint64_t totalTasks() const { return total_enqueued_; }
    uint64_t completedTasks() const { return total_completed_; }
    size_t pendingTasks() const;
    size_t activeThreads() const;

    void shutdown();

private:
    void worker_loop();
    void addWorker();
    
    // 修改: 添加移除工作线程方法
    void removeWorker(size_t count = 1);

    std::vector<std::thread> workers_;
    std::priority_queue<Task> tasks_;

    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{ false };

    std::atomic<uint64_t> total_enqueued_{0};
    std::atomic<uint64_t> total_completed_{0};
    std::atomic<size_t> active_thread_count_{0};
    
    // 修改: 用vector<bool>替代atomic<bool>的vector
    std::vector<bool> worker_exit_flags_;
    // 存储线程ID到索引的映射
    std::mutex thread_map_mutex_;
    std::unordered_map<std::thread::id, size_t> thread_to_index_;
};