// ev_thread.h - 修复版

#pragma once

#include "ev_thread_manager.h"
#include <future>
#include <memory>
#include <atomic>
#include "common.h"
#include <utility>

// 线程封装类，支持用future异步获取返回值、线程优先级、超时等
class EVThread {
public:
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
        
        // 直接move func和args进lambda
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [func = std::move(func), args = std::move(args)]() mutable {
                return std::apply(std::move(func), std::move(args));
            }
        );
        std::future<ReturnType> future = task->get_future();

        auto runner = [task, thread_id_out]() mutable {
            ThreadLogs tlogs;
            tlogs.start_time = std::chrono::system_clock::now();
            tlogs.thread_id = std::this_thread::get_id();
            tlogs.thread_state = ThreadState::RUNNING;
            if (thread_id_out) {
                *thread_id_out = std::this_thread::get_id();
            }
            try {
                (*task)();
                tlogs.thread_state = ThreadState::COMPLETED;
            } catch (const std::exception& e) {
                tlogs.thread_state = ThreadState::FAILED;
                tlogs.last_error = e.what();
                LOG(ERROR) << "EVThread failed: " << e.what();
            } catch (...) {
                tlogs.thread_state = ThreadState::FAILED;
                tlogs.last_error = "Unknown exception";
                LOG(ERROR) << "EVThread failed: unknown exception";
            }
            tlogs.end_time = std::chrono::system_clock::now();
            EVThreadManager::instance().addThreadLog(tlogs);
        };

        // 修复: 创建线程并使用智能指针管理
        auto thread_ptr = std::make_shared<std::thread>(runner);
        
        // 修复: 尝试设置线程优先级，但忽略错误（记录但继续执行）
        try {
            setThreadPriority(*thread_ptr, priority);
        } catch (const std::exception& e) {
            LOG(WARNING) << "Thread priority setting failed: " << e.what() << " (继续执行)";
        }

        // 修复: 改进的超时处理
        if (timeout.count() > 0) {
            // 创建监控线程来处理超时
            std::thread([timeout, t = std::move(thread_ptr)]() mutable {
                if (!t->joinable()) return;
                
                // 尝试在超时前等待线程完成
                auto join_result = joinWithTimeout(*t, timeout);
                
                // 如果超时，安全地分离线程
                if (!join_result && t->joinable()) {
                    LOG(WARNING) << "Thread join timeout after " << timeout.count() << "ms: detaching";
                    t->detach();
                }
            }).detach();
        }

        return future;
    }

    // 修复: 添加带超时的join方法
    static bool joinWithTimeout(std::thread& thread, std::chrono::milliseconds timeout) {
        if (!thread.joinable()) return false;
        
        if (timeout.count() <= 0) {
            thread.join();
            return true;
        }

        // 使用C++11的std::condition_variable实现超时等待
        std::mutex mtx;
        std::condition_variable cv;
        bool finished = false;
        
        // 创建监控线程
        std::thread waiter([&thread, &finished, &cv]() {
            if (thread.joinable()) {
                thread.join();
            }
            finished = true;
            cv.notify_one();
        });
        
        // 等待完成或超时
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (cv.wait_for(lock, timeout, [&finished]() { return finished; })) {
                // 线程在超时前完成
                if (waiter.joinable()) {
                    waiter.join();
                }
                return true;
            }
        }
        
        // 超时
        if (waiter.joinable()) {
            waiter.detach();
        }
        return false;
    }

    // 等待线程结束，支持超时
    static bool joinThread(std::thread& thread, std::chrono::milliseconds timeout = std::chrono::milliseconds{0});
    static void detachThread(std::thread& thread);
    static bool isRunning(const std::thread& thread);

    // 修复: 设置线程优先级（平台相关），增加错误处理
    static bool setThreadPriority(std::thread& thread, ThreadPriority priority);

    // 查询线程日志
    static bool getThreadLogs(std::thread::id tid, ThreadLogs& out_log);
};