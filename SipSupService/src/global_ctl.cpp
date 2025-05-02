// global_ctl.cpp

#include "global_ctl.h"
#include "sip_core.h"

#include <algorithm>

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

    // 构建域信息列表
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
    const auto& nodes = g_config_->getNodeInfoList();
    // 使用写锁保护域信息映射表
    std::unique_lock<std::shared_mutex> lock(domain_mutex_);
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
    LOG(INFO) << "Checking if domain is valid: " << id;
    // 使用读锁访问域信息映射表
    std::shared_lock<std::shared_mutex> lock(domain_mutex_);
    return std::any_of(
        domain_info_list_.begin(),
        domain_info_list_.end(),
        [&id](const DomainInfo& domain){
            return domain.sip_id == id && domain.registered;
        }
    );
}

DomainInfo* GlobalCtl::findDomain(std::string_view id)
{
    auto it = std::find_if(
        domain_info_list_.begin(),
        domain_info_list_.end(),
        [&id](const DomainInfo& domain) {
            return domain.sip_id == id;
        }
    );
    return it != domain_info_list_.end() 
        ? &(*it) 
        : nullptr; 
}

void GlobalCtl::setExpires(std::string_view id, int expires_value) 
{

    auto domain = findDomain(id);
    if (domain) 
    {
        domain->expires = expires_value;
        LOG(INFO) << "Updated expires for domain: " << id 
            << " to " << expires_value;
    } 
    else 
    {
        LOG(ERROR) << "Domain not found: " << id;
    }
}

void GlobalCtl::setRegistered(std::string_view id, bool registered_value) 
{
    auto domain = findDomain(id);
    if (domain) 
    {
        domain->registered = registered_value;
        LOG(INFO) << "Updated registered status for domain: " << id 
            << " to " << registered_value;
    } 
    else 
    {
        LOG(ERROR) << "Domain not found: " << id;
    }
}

void GlobalCtl::setLastRegTime(std::string_view id, time_t last_reg_time_value) 
{
    std::unique_lock<std::shared_mutex> lock(domain_mutex_);  // 写锁
    auto domain = findDomain(id);
    if (domain) 
    {
        domain->last_reg_time = last_reg_time_value;
        LOG(INFO) << "Updated last registration time for domain: " << id << " to " << last_reg_time_value;
    } 
    else 
    {
        LOG(ERROR) << "Domain not found: " << id;
    }
}

// 批量原子更新接口
void GlobalCtl::updateRegistration(std::string_view id, int expires_new, bool registered_new, time_t last_reg_time_new)
{
    // 使用写锁保护注册信息映射表
    std::unique_lock<std::shared_mutex> lock(register_mutex_);
    
    auto domain = findDomain(id);
    if (domain)
    {
        domain->expires = expires_new;
        domain->registered = registered_new;
        domain->last_reg_time = last_reg_time_new;
        LOG(INFO) << "updateRegistration: " << id 
            << " expires=" << expires_new 
            << " registered=" << registered_new 
            << " last_reg_time=" << last_reg_time_new;
    }
    else
    {
        LOG(ERROR) << "updateRegistration: Domain not found: " << id;
    }
    update_counter_++;
}