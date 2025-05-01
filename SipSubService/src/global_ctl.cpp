// global_ctl.cpp

#include "global_ctl.h"
#include "sip_core.h"



bool GlobalCtl::init(std::unique_ptr<IConfigProvider> config) 
{
    LOG(INFO) << "GlobalCtl instance init..."; 
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
        g_sip_core_ = std::make_shared<SipCore>();
    }
    
    g_sip_core_->initSip(g_config_->getSipPort());
    LOG(INFO) << "GlobalCtl instance init success!";
    return true;
}

void GlobalCtl::buildDomainInfoList()
{
    LOG(INFO) << "Building DomainInfo list...";
    std::unique_lock<std::shared_mutex> lock(domain_mutex_);  // 写锁
    const auto& nodes = g_config_->getNodeInfoList();
    domain_info_list_.clear();
    domain_info_list_.reserve(nodes.size());

    for(const auto& node : nodes)
    {
        domain_info_list_.emplace_back(node);
    }
    LOG(INFO) << "Built " << domain_info_list_.size() << " domain entries";
}

