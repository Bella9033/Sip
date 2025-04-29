//global_ctl.h

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

// 前向声明
class SipLocalConfig;

// 全局控制器单例，现代C++智能指针风格
class GlobalCtl : public IDomainManager
{
public:
    // 单例访问点
    static GlobalCtl& getInstance() 
    { 
        static GlobalCtl instance; 
        return instance; 
    }

    // 初始化方法 - 接收配置提供者实现
    bool init(std::unique_ptr<IConfigProvider> config);

    // 获取配置接口
    const IConfigProvider& getConfig() const { return *g_config_; }
    
    // 其他访问器
    const ThreadPool& getThreadPool() const { return *g_thread_pool_; }
    ISipCore& getSipCore() const { return *g_sip_core_; }

    // 实现IDomainManager接口
    void buildDomainInfoList() override;
    bool checkIsValid(const std::string& id) const override;
    void setExpires(std::string_view id, int expires) override;
    std::vector<DomainInfo>& getDomainInfoList() override { return domain_info_list_; }
    
    std::mutex reg_mutex_;
    
private:
    GlobalCtl() = default;
    ~GlobalCtl() = default;
    GlobalCtl(const GlobalCtl&) = delete;
    GlobalCtl& operator=(const GlobalCtl&) = delete;

    std::mutex g_init_mutex_;
    mutable std::mutex domain_mutex_; // 允许在 const 方法中使用

    std::unique_ptr<IConfigProvider> g_config_; // 修改为接口类型
    std::unique_ptr<ThreadPool> g_thread_pool_;
    std::shared_ptr<ISipCore> g_sip_core_;

    std::vector<DomainInfo> domain_info_list_;
};