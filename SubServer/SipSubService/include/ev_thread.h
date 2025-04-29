// ev_thread.h

#pragma once

#include "ev_thread_manager.h"
#include <future>
#include <memory>
#include <atomic>
#include "common.h"
#include <utility>

// 线程封装类，支持用future异步获取返回值、线程优先级、超时等
// 模板实现必须放头文件，否则链接时会出现未定义错误（模板实例化规则）。
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

        std::thread thd(runner);
        setThreadPriority(thd, priority);

        // ev_thread.h 中 createThread 方法的关键片段修改
    if (timeout.count() > 0) {
        // 超时后 detach，但此时 future.get() 就不要用了
        std::thread([timeout, t = std::move(thd)]() mutable {
            auto start = std::chrono::steady_clock::now();
            
            // 增加检查次数和间隔时间
            const int MAX_CHECKS = 20; // 检查次数上限
            int check_count = 0;
            
            while (t.joinable() && 
                std::chrono::steady_clock::now() - start < timeout && 
                check_count < MAX_CHECKS) {
                // 增加睡眠时间，减少频繁检查
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
                check_count++;
            }
            
            if (t.joinable()) {
                try {
                    LOG(WARNING) << "EVThread join timeout: thread will be detached.";
                    t.detach();
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Exception during thread detach: " << e.what();
                }
            }
        }).detach();
    }


        return future;
    }

    // 等待线程结束，支持超时
    static bool joinThread(std::thread& thread, std::chrono::milliseconds timeout = std::chrono::milliseconds{0});
    static void detachThread(std::thread& thread);
    static bool isRunning(const std::thread& thread);

    // 设置线程优先级（平台相关）
    static bool setThreadPriority(std::thread& thread, ThreadPriority priority);

    // 查询线程日志
    static bool getThreadLogs(std::thread::id tid, ThreadLogs& out_log);

    // 危险接口，强制终止线程（不推荐实际用）
    static void terminateThread(std::thread& thread);
};