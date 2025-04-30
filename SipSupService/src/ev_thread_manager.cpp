// ev_thread_manager.cpp

#include "ev_thread_manager.h"

EVThreadManager& EVThreadManager::instance() {
    static EVThreadManager inst;
    return inst;
}

void EVThreadManager::addThreadLog(const ThreadLogs& log) {
    std::lock_guard<std::mutex> lock(mutex_);
    threads_[log.thread_id] = log;
}

bool EVThreadManager::getThreadLog(std::thread::id tid, ThreadLogs& out_log) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = threads_.find(tid);
    if (it != threads_.end()) {
        out_log = it->second;
        return true;
    }
    return false;
}