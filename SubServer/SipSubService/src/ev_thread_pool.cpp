// ev_thread_pool.cpp

#include "ev_thread_pool.h"
#include "common.h"

ThreadPool::ThreadPool(size_t thread_count)
{
    if (thread_count == 0) throw std::invalid_argument("thread_count must be > 0");
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
    }
    condition_.notify_all();
    for (auto& w : workers_)
    {
        if (w.joinable())
            w.join();
    }
    LOG(INFO) << "ThreadPool shutdown complete.";
}

void ThreadPool::addWorker()
{
    workers_.emplace_back([this] { this->worker_loop(); });
}

void ThreadPool::worker_loop()
{
    while (true)
    {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty())
            {
                return;
            }
            task = std::move(const_cast<Task&>(tasks_.top()));
            tasks_.pop();
            ++active_thread_count_;
        }
        try
        {
            task.func();
            ++total_completed_;
        }
        catch (const std::exception& e)
        {
            LOG(ERROR) << "Exception in thread pool task: " << e.what();
        }
        catch (...)
        {
            LOG(ERROR) << "Unknown exception in thread pool task.";
        }
        --active_thread_count_;
    }
}