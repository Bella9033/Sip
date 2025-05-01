// ev_thread.h - 修改版
#pragma once

#include <thread>
#include <future>
#include <chrono>
#include <memory>
#include <atomic>
#include <functional>
#include <type_traits>
#include "common.h"
#include "ev_thread_pool.h" // 引入线程池头文件

// 前置声明
class ThreadPool;

// 线程优先级
enum class ThreadPriority { LOW, NORMAL, HIGH };

// 线程运行状态
enum class ThreadState { CREATED, RUNNING, COMPLETED, FAILED };

// 线程状态跟踪
struct ThreadLogs {
    std::thread::id thread_id;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    ThreadState thread_state{ThreadState::CREATED};
    std::string last_error;
};

// 线程封装类
class EVThread {
public:
    // 获取全局线程池的实例
    static ThreadPool& getThreadPool();
    
    // 创建并启动新线程，支持参数包及优先级、超时设置
    template <typename Func, typename... Args>
    static auto createThread(
        Func&& func,
        std::tuple<Args...> args,
        std::shared_ptr<std::atomic<std::thread::id>> thread_id_out = nullptr,
        ThreadPriority priority = ThreadPriority::NORMAL,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{0}
    ) -> std::future<std::invoke_result_t<Func, Args...>>
    {
        using ReturnType = std::invoke_result_t<Func, Args...>;
        
        // 创建一个共享的任务对象
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [func = std::forward<Func>(func), args = std::move(args)]() mutable {
                return std::apply(std::move(func), std::move(args));
            }
        );
        
        // 获取future用于返回结果
        std::future<ReturnType> future = task->get_future();
        
        // 创建执行任务的lambda
        auto thread_func = [task, thread_id_out]() {
            ThreadLogs logs;
            logs.start_time = std::chrono::system_clock::now();
            logs.thread_id = std::this_thread::get_id();
            logs.thread_state = ThreadState::RUNNING;
            
            // 如果需要，输出线程ID
            if (thread_id_out) {
                *thread_id_out = std::this_thread::get_id();
            }
            
            try {
                // 执行任务
                (*task)();
                logs.thread_state = ThreadState::COMPLETED;
            } catch (const std::exception& e) {
                logs.thread_state = ThreadState::FAILED;
                logs.last_error = e.what();
                LOG(ERROR) << "Thread execution failed: " << e.what();
            } catch (...) {
                logs.thread_state = ThreadState::FAILED;
                logs.last_error = "Unknown exception";
                LOG(ERROR) << "Thread execution failed with unknown exception";
            }
            
            logs.end_time = std::chrono::system_clock::now();
            // 存储线程日志
            storeThreadLog(logs);
        };
        
        // 创建新线程并启动
        // 判断是否使用线程池
        if (shouldUseThreadPool()) {
            // 任务提交给线程池处理，优先级基于ThreadPriority
            int task_priority = getPriorityValue(priority);
            getThreadPool().submit(task_priority, thread_func);
        } else {
            // 直接创建线程
            std::shared_ptr<std::thread> thread_ptr = std::make_shared<std::thread>(thread_func);
            
            // 尝试设置优先级（忽略失败）
            try {
                setThreadPriority(*thread_ptr, priority);
            } catch (const std::exception& e) {
                LOG(WARNING) << "Failed to set thread priority: " << e.what();
            }
            
            // 如果设置了超时，创建监控线程
            if (timeout.count() > 0) {
                std::thread monitor_thread([timeout, thread = thread_ptr]() {
                    if (joinWithTimeout(*thread, timeout)) {
                        LOG(INFO) << "Thread completed within timeout";
                    } else {
                        LOG(WARNING) << "Thread timed out after " << timeout.count() << "ms";
                        // 我们不会强制终止线程，只记录超时
                    }
                });
                monitor_thread.detach(); // 监控线程可以分离
            } else {
                // 如果没有超时，创建waiter线程等待完成
                std::thread waiter([thread = thread_ptr]() {
                    if (thread->joinable()) {
                        thread->join();
                        LOG(INFO) << "Thread joined successfully";
                    }
                });
                waiter.detach(); // waiter线程可以分离
            }
        }
        
        return future;
    }
    
    // 带超时的线程等待
    static bool joinWithTimeout(std::thread& thread, std::chrono::milliseconds timeout);
    
    // 安全地分离线程
    static void detachThread(std::thread& thread);
    
    // 检查线程是否在运行
    static bool isRunning(const std::thread& thread);
    
    // 设置线程优先级
    static bool setThreadPriority(std::thread& thread, ThreadPriority priority);
    
    // 线程池配置
    static void enableThreadPool(bool enable);
    static bool isThreadPoolEnabled();

private:
    // 存储线程日志
    static void storeThreadLog(const ThreadLogs& logs);
    
    // 决定是否应该使用线程池
    static bool shouldUseThreadPool();
    
    // 将ThreadPriority转换为线程池优先级值
    static int getPriorityValue(ThreadPriority priority);
    
    // 控制是否使用线程池
    static std::atomic<bool> use_thread_pool_;
};