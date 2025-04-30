// global_ctl.cpp

#include "global_ctl.h"
#include "sip_core.h"

#include <algorithm>

bool GlobalCtl::init(std::unique_ptr<IConfigProvider> config) 
{
    LOG(INFO) << "GlobalCtl instance init...";
    std::lock_guard<std::mutex> lock(g_init_mutex_);
    
    // 保存配置提供者
    g_config_ = std::move(config);
    
    if (!g_config_) 
    {
        LOG(ERROR) << "GlobalCtl instance init failed: no config provider!";
        return false;
    }
    
    if(!g_config_->readConf()) 
    {
        LOG(ERROR) << "GlobalCtl instance readConfig failed!";
        return false;
    }
    
    buildDomainInfoList();
    if (domain_info_list_.empty()) 
    {
        LOG(ERROR) << "GlobalCtl instance buildDomainInfoList failed!";
        return false;
    }

    if (!g_thread_pool_) 
    {
        g_thread_pool_ = std::make_unique<ThreadPool>(4);
    }
    
    if (!g_sip_core_) 
    {
        // 创建具体实现类，但存储为接口类型
        g_sip_core_ = std::make_shared<SipCore>();
    }
    
    g_sip_core_->initSip(g_config_->getSipPort());
    LOG(INFO) << "GlobalCtl instance init success!";
    return true;
}

void GlobalCtl::buildDomainInfoList()
{
    LOG(INFO) << "Building DomainInfo list...";
    std::lock_guard<std::mutex>lock(domain_mutex_);
    
    // 通过接口获取NodeInfo列表
    const auto& nodes = g_config_->getNodeInfoList();
    
    domain_info_list_.clear();
    domain_info_list_.reserve(nodes.size());

    for(const auto& node : nodes)
    {
        domain_info_list_.emplace_back(node);
    }
    
    LOG(INFO) << "Built " << domain_info_list_.size() << " domain entries";
}

bool GlobalCtl::checkIsValid(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(domain_mutex_);
    return std::any_of(
        domain_info_list_.begin(),
        domain_info_list_.end(),
        [&id](const DomainInfo& domains){
            return domains.sip_id == id;
        }
    );
}

void GlobalCtl::setExpires(std::string_view id, int expires)
{
    std::lock_guard<std::mutex> lock(domain_mutex_);
    if(auto it = std::find_if(
        domain_info_list_.begin(), domain_info_list_.end(),
        [&id](const DomainInfo& domains){
            return domains.sip_id == id;
        });
        it != domain_info_list_.end())
    {
        it->expires = expires;
        LOG(INFO) << fmt::format("Set expires for ID {} to {}", id, expires);
    }
}