// global_ctl.h

#pragma once

#include "common.h"
#include "ev_thread_pool.h"
#include "interfaces/isip_core.h"
#include "interfaces/iconfig_provider.h"
#include "interfaces/idomain_manager.h"
#include "node_info.h"

#include <memory>
#include <mutex>
#include <vector>
#include <string_view>
#include <shared_mutex>
#include <atomic>

class SipLocalConfig;

class GlobalCtl : public IDomainManager
{
public:
    static GlobalCtl& getInstance() 
    { 
        static GlobalCtl instance; 
        return instance; 
    }

    bool init(std::unique_ptr<IConfigProvider> config);

    const IConfigProvider& getConfig() const { return *g_config_; }
    const ThreadPool& getThreadPool() const { return *g_thread_pool_; }
    ISipCore& getSipCore() const { return *g_sip_core_; }

    void buildDomainInfoList() override;

    std::shared_mutex& getMutex() override { return domain_mutex_; }// 获取互斥锁

    std::vector<DomainInfo>& getDomainInfoList() override { return domain_info_list_; }

   
private:
    GlobalCtl() = default;
    ~GlobalCtl() = default;
    GlobalCtl(const GlobalCtl&) = delete;
    GlobalCtl& operator=(const GlobalCtl&) = delete;

    // 使用读写锁替代互斥锁，提高并发性
    mutable std::shared_mutex domain_mutex_; 
    // 添加原子操作计数器
    std::atomic<size_t> update_counter_{0};

    std::unique_ptr<IConfigProvider> g_config_;
    std::unique_ptr<ThreadPool> g_thread_pool_;
    std::shared_ptr<ISipCore> g_sip_core_;

    std::vector<DomainInfo> domain_info_list_;
    

};