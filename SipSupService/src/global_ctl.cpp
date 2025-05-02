#include "global_ctl.h"
#include "sip_core.h"
#include <algorithm>

bool GlobalCtl::init(std::unique_ptr<IConfigProvider> config) {
    LOG(INFO) << "GlobalCtl instance init...";
    g_config_ = std::move(config);
    
    if (!g_config_ || !g_config_->readConf()) {
        LOG(ERROR) << "GlobalCtl config initialization failed";
        return false;
    }

    buildDomainInfoList();
    if (domain_info_list_.empty()) {
        LOG(ERROR) << "Domain info list is empty";
        return false;
    }

    g_thread_pool_ = std::make_unique<ThreadPool>(4);
    g_sip_core_ = SipCore::create();
    
    if (!g_sip_core_ || !g_thread_pool_) {
        LOG(ERROR) << "Failed to create core components";
        return false;
    }
    
    if (g_sip_core_->initSip(g_config_->getSipPort()) != PJ_SUCCESS) {
        LOG(ERROR) << "Failed to initialize SIP core";
        return false;
    }

    LOG(INFO) << "GlobalCtl instance initialized successfully";
    return true;
}

void GlobalCtl::buildDomainInfoList() {
    LOG(INFO) << "Building DomainInfo list...";
    std::unique_lock<std::shared_mutex> lock(domain_mutex_);
    const auto& nodes = g_config_->getNodeInfoList();
    domain_info_list_.clear();
    domain_info_list_.reserve(nodes.size());

    for(const auto& node : nodes) {
        domain_info_list_.emplace_back(node);
    }
    LOG(INFO) << "Built " << domain_info_list_.size() << " domain entries";
}

bool GlobalCtl::checkIsValid(const std::string& id) const {
    std::shared_lock<std::shared_mutex> lock(domain_mutex_);
    return std::any_of(
        domain_info_list_.begin(),
        domain_info_list_.end(),
        [&id](const DomainInfo& domain) {
            return domain.sip_id == id && domain.registered;
        }
    );
}

DomainInfo* GlobalCtl::findDomain(std::string_view id) {
    auto it = std::find_if(
        domain_info_list_.begin(),
        domain_info_list_.end(),
        [&id](const DomainInfo& domain) {
            return domain.sip_id == id;
        }
    );
    return it != domain_info_list_.end() ? &(*it) : nullptr;
}

void GlobalCtl::setExpires(std::string_view id, int expires_value) {
    std::unique_lock<std::shared_mutex> lock(domain_mutex_);
    auto domain = findDomain(id);
    if (domain) {
        domain->expires = expires_value;
        LOG(INFO) << "Updated expires for domain: " << id 
                 << " to " << expires_value;
    } else {
        LOG(ERROR) << "Domain not found: " << id;
    }
}

void GlobalCtl::setRegistered(std::string_view id, bool registered_value) {
    std::unique_lock<std::shared_mutex> lock(domain_mutex_);
    auto domain = findDomain(id);
    if (domain) {
        domain->registered = registered_value;
        LOG(INFO) << "Updated registered status for domain: " << id 
                 << " to " << registered_value;
    } else {
        LOG(ERROR) << "Domain not found: " << id;
    }
}

void GlobalCtl::setLastRegTime(std::string_view id, time_t last_reg_time_value) {
    std::unique_lock<std::shared_mutex> lock(domain_mutex_);
    auto domain = findDomain(id);
    if (domain) {
        domain->last_reg_time = last_reg_time_value;
        LOG(INFO) << "Updated last registration time for domain: " << id 
                 << " to " << last_reg_time_value;
    } else {
        LOG(ERROR) << "Domain not found: " << id;
    }
}

void GlobalCtl::updateRegistration(std::string_view id, 
                                 int expires_value,
                                 bool is_registered,
                                 time_t reg_time) {
    std::unique_lock<std::shared_mutex> lock(register_mutex_);
    auto domain = findDomain(id);
    if (domain) {
        domain->expires = expires_value;
        domain->registered = is_registered;
        domain->last_reg_time = reg_time;
        LOG(INFO) << "Updated registration for domain: " << id
                 << " expires=" << expires_value
                 << " registered=" << is_registered
                 << " last_reg_time=" << reg_time;
    } else {
        LOG(ERROR) << "Domain not found for registration update: " << id;
    }
    update_counter_++;
}