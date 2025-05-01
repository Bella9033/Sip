// idomain_manager.h

#pragma once

#include "node_info.h"
#include <string>
#include <string_view>
#include <vector>
#include <ctime>
#include <shared_mutex>

// 域名管理接口
class IDomainManager 
{
public:
    virtual ~IDomainManager() noexcept = default;
    virtual void buildDomainInfoList() = 0;
    virtual bool checkIsValid(const std::string& id) const = 0;
    virtual std::vector<DomainInfo>& getDomainInfoList() = 0;

    virtual std::shared_mutex& getMutex() = 0; // 获取互斥锁
    virtual DomainInfo* findDomain(std::string_view id) = 0;


    virtual void setExpires(std::string_view id, int expires_value) = 0;
    virtual void setRegistered(std::string_view id, bool registered_value) = 0;
    virtual void setLastRegTime(std::string_view id, time_t last_reg_time_value) = 0;

    // 批量原子更新接口
    virtual void updateRegistration(std::string_view id, int expires_new, bool registered_new, time_t last_reg_time_new) = 0;
};