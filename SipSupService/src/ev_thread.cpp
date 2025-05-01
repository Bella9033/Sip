// ev_thread.cpp - 修改版
#include "ev_thread.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

// 平台特定的头文件
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// 初始化静态成员
std::atomic<bool> EVThread::use_thread_pool_(true); // 默认使用线程池

// 线程日志存储
namespace {
    std::mutex logs_mutex;
    std::unordered_map<std::thread::id, ThreadLogs> thread_logs;
}

// 获取全局线程池实例
ThreadPool& EVThread::getThreadPool() {
    // 使用局部静态变量作为单例模式
    static ThreadPool pool(std::thread::hardware_concurrency());
    return pool;
}

bool EVThread::shouldUseThreadPool() {
    return use_thread_pool_.load();
}

void EVThread::enableThreadPool(bool enable) {
    use_thread_pool_.store(enable);
}

bool EVThread::isThreadPoolEnabled() {
    return use_thread_pool_.load();
}

int EVThread::getPriorityValue(ThreadPriority priority) {
    switch (priority) {
        case ThreadPriority::LOW:    return 0;
        case ThreadPriority::NORMAL: return 5;
        case ThreadPriority::HIGH:   return 10;
        default:                     return 5;
    }
}

bool EVThread::joinWithTimeout(std::thread& thread, std::chrono::milliseconds timeout) {
    if (!thread.joinable()) {
        LOG(WARNING) << "Thread is not joinable";
        return false;
    }
    
    if (timeout.count() <= 0) {
        // 没有超时，直接join
        thread.join();
        return true;
    }
    
    // 使用条件变量实现超时等待
    std::mutex mtx;
    std::condition_variable cv;
    bool finished = false;
    
    // 创建辅助线程进行join
    std::thread waiter([&thread, &finished, &cv]() {
        if (thread.joinable()) {
            thread.join();
            finished = true;
            cv.notify_one();
        }
    });
    
    // 等待join完成或超时
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (cv.wait_for(lock, timeout, [&finished]() { return finished; })) {
            // 在超时前完成
            waiter.join();
            return true;
        }
    }
    
    // 超时，分离辅助线程
    waiter.detach();
    return false;
}

void EVThread::detachThread(std::thread& thread) {
    if (thread.joinable()) {
        thread.detach();
        LOG(INFO) << "Thread detached";
    }
}

bool EVThread::isRunning(const std::thread& thread) {
    return thread.joinable();
}

bool EVThread::setThreadPriority(std::thread& thread, ThreadPriority priority) {
    try {
#ifdef _WIN32
        // Windows实现
        auto handle = thread.native_handle();
        int win_priority = THREAD_PRIORITY_NORMAL;
        
        switch (priority) {
            case ThreadPriority::LOW:    win_priority = THREAD_PRIORITY_BELOW_NORMAL; break;
            case ThreadPriority::NORMAL: win_priority = THREAD_PRIORITY_NORMAL; break;
            case ThreadPriority::HIGH:   win_priority = THREAD_PRIORITY_ABOVE_NORMAL; break;
        }
        
        if (!SetThreadPriority(handle, win_priority)) {
            LOG(WARNING) << "Failed to set thread priority on Windows";
            return false;
        }
#else
        // POSIX实现
        auto handle = thread.native_handle();
        int policy;
        struct sched_param param;
        
        // 获取当前参数
        if (pthread_getschedparam(handle, &policy, &param) != 0) {
            LOG(WARNING) << "Failed to get thread scheduling parameters";
            return false;
        }
        
        // 获取优先级范围
        int min_prio = sched_get_priority_min(policy);
        int max_prio = sched_get_priority_max(policy);
        
        if (min_prio == -1 || max_prio == -1) {
            LOG(WARNING) << "Cannot determine thread priority range";
            return false;
        }
        
        // 设置新的优先级
        int prio_range = max_prio - min_prio;
        switch (priority) {
            case ThreadPriority::LOW:
                param.sched_priority = min_prio;
                break;
            case ThreadPriority::NORMAL:
                param.sched_priority = min_prio + (prio_range / 2);
                break;
            case ThreadPriority::HIGH:
                param.sched_priority = max_prio;
                break;
        }
        
        // 应用设置
        if (pthread_setschedparam(handle, policy, &param) != 0) {
            LOG(WARNING) << "Failed to set thread scheduling parameters";
            return false;
        }
#endif
        return true;
    } catch (const std::exception& e) {
        LOG(WARNING) << "Exception when setting thread priority: " << e.what();
        return false;
    }
}

void EVThread::storeThreadLog(const ThreadLogs& logs) {
    std::lock_guard<std::mutex> lock(logs_mutex);
    thread_logs[logs.thread_id] = logs;
}