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

class GlobalCtl : public IDomainManager {
public:
    static GlobalCtl& getInstance() { 
        static GlobalCtl instance; 
        return instance; 
    }

    bool init(std::unique_ptr<IConfigProvider> config);

    // Getters
    const IConfigProvider& getConfig() const { return *g_config_; }
    const ThreadPool& getThreadPool() const { return *g_thread_pool_; }
    ISipCore& getSipCore() const { return *g_sip_core_; }

    // IDomainManager接口实现
    void buildDomainInfoList() override;
    bool checkIsValid(const std::string& id) const override;
    DomainInfo* findDomain(std::string_view id) override;
    std::vector<DomainInfo>& getDomainInfoList() override { return domain_info_list_; }
    
    // 注册状态管理实现
    void setExpires(std::string_view id, int expires_value) override;
    void setRegistered(std::string_view id, bool registered_value) override;
    void setLastRegTime(std::string_view id, time_t last_reg_time_value) override;
    void updateRegistration(std::string_view id, 
                          int expires_value,
                          bool is_registered,
                          time_t reg_time) override;

    // 线程安全实现
    std::shared_mutex& getMutex() override { return domain_mutex_; }

private:
    GlobalCtl() = default;
    ~GlobalCtl() = default;
    GlobalCtl(const GlobalCtl&) = delete;
    GlobalCtl& operator=(const GlobalCtl&) = delete;

    mutable std::shared_mutex domain_mutex_;
    mutable std::shared_mutex register_mutex_;
    std::atomic<size_t> update_counter_{0};

    std::unique_ptr<IConfigProvider> g_config_;
    std::unique_ptr<ThreadPool> g_thread_pool_;
    std::shared_ptr<ISipCore> g_sip_core_;
    std::vector<DomainInfo> domain_info_list_;
};