// ev.thread.cpp

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
    if (!thread.joinable()) { return false; }
    if (timeout.count() <= 0) 
    {
        thread.join();
        return true;
    }
    std::atomic<bool> done{false};
    std::mutex mtx;
    std::condition_variable cv;
    std::thread waiter([&]() 
    {
        if (thread.joinable()) thread.join();
        done = true;
        cv.notify_one();
    });
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (!cv.wait_for(lock, timeout, [&] { return done.load(); })) 
        {
            if (waiter.joinable()) { waiter.detach();}
            return false;
        }
    }
    if (waiter.joinable()) { waiter.join(); }
    return true;
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
    try {
        auto handle = thread.native_handle();
        int policy;
        struct sched_param param;
        if (pthread_getschedparam(handle, &policy, &param) != 0) {
            LOG(ERROR) << "Failed to get thread scheduling parameters";
            return false;
        }
        switch (priority) {
            case ThreadPriority::LOW:    param.sched_priority = 0; break;
            case ThreadPriority::NORMAL: param.sched_priority = 1; break;
            case ThreadPriority::HIGH:   param.sched_priority = 2; break;
        }
        if (pthread_setschedparam(handle, policy, &param) != 0) {
            LOG(ERROR) << "Failed to set thread scheduling parameters";
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Exception while setting thread priority: " << e.what();
        return false;
    }
}

bool EVThread::getThreadLogs(std::thread::id tid, ThreadLogs& out_log) 
{
    return EVThreadManager::instance().getThreadLog(tid, out_log);
}


