// ev_thread.cpp - 修改版

#include "ev_thread.h"
#include <thread>
#include <future>
#include <condition_variable>
#include <chrono>
#include <exception>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// join/detach/isRunning/优先级设置等实现
bool EVThread::joinThread(std::thread& thread, std::chrono::milliseconds timeout) 
{
    return joinWithTimeout(thread, timeout);
}

void EVThread::detachThread(std::thread& thread) 
{
    if (thread.joinable()) 
    {
        thread.detach();
        LOG(INFO) << "Thread " << thread.get_id() << " detached.";
    }
}

bool EVThread::isRunning(const std::thread& thread) 
{
    return thread.joinable();
}

bool EVThread::setThreadPriority(std::thread& thread, ThreadPriority priority) 
{
    // 修改: 增加更健壮的错误处理
    try {
#ifdef _WIN32
        // Windows平台实现
        auto handle = thread.native_handle();
        int win_priority = THREAD_PRIORITY_NORMAL;
        
        switch (priority) {
            case ThreadPriority::LOW:    win_priority = THREAD_PRIORITY_BELOW_NORMAL; break;
            case ThreadPriority::NORMAL: win_priority = THREAD_PRIORITY_NORMAL; break;
            case ThreadPriority::HIGH:   win_priority = THREAD_PRIORITY_ABOVE_NORMAL; break;
        }
        
        if (!SetThreadPriority(handle, win_priority)) {
            LOG(WARNING) << "Failed to set Windows thread priority, continuing with default";
            // 非关键错误，返回true继续执行
            return true;
        }
#else
        // POSIX平台实现
        auto handle = thread.native_handle();
        int policy;
        struct sched_param param;
        
        // 获取当前调度策略和参数
        if (pthread_getschedparam(handle, &policy, &param) != 0) {
            LOG(WARNING) << "Failed to get thread scheduling parameters, continuing with default";
            // 非关键错误，返回true继续执行
            return true;
        }
        
        // 修改: 改进优先级映射，避免超过系统限制
        int min_prio = sched_get_priority_min(policy);
        int max_prio = sched_get_priority_max(policy);
        
        if (min_prio == -1 || max_prio == -1) {
            LOG(WARNING) << "Cannot determine thread priority range, continuing with default";
            return true;
        }
        
        // 计算相对优先级
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
        
        // 尝试设置新的优先级
        int result = pthread_setschedparam(handle, policy, &param);
        if (result != 0) {
            // 这可能是因为权限问题导致的失败，不应中断程序执行
            LOG(WARNING) << "Failed to set thread scheduling parameters, code: " << result 
                         << " (可能需要root权限或CAP_SYS_NICE能力)";
            return true; // 返回true允许继续执行
        }
#endif
        return true;
    } catch (const std::exception& e) {
        LOG(WARNING) << "Exception while setting thread priority: " << e.what() << " (continuing)";
        return true; // 非关键错误，允许程序继续
    }
}

bool EVThread::getThreadLogs(std::thread::id tid, ThreadLogs& out_log) 
{
    return EVThreadManager::instance().getThreadLog(tid, out_log);
}