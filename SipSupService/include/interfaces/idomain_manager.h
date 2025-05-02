#pragma once

#include "common.h"
#include "node_info.h"
#include <shared_mutex>
#include <string_view>

class IDomainManager {
public:
    virtual ~IDomainManager() = default;
    
    // 基本域管理接口
    virtual void buildDomainInfoList() = 0;
    virtual bool checkIsValid(const std::string& id) const = 0;
    virtual DomainInfo* findDomain(std::string_view id) = 0;
    virtual std::vector<DomainInfo>& getDomainInfoList() = 0;
    
    // 注册状态管理接口
    virtual void setExpires(std::string_view id, int expires_value) = 0;
    virtual void setRegistered(std::string_view id, bool registered_value) = 0;
    virtual void setLastRegTime(std::string_view id, time_t last_reg_time_value) = 0;
    virtual void updateRegistration(std::string_view id, 
                                  int expires_value,
                                  bool is_registered,
                                  time_t reg_time) = 0;

    // 线程安全接口
    virtual std::shared_mutex& getMutex() = 0;

protected:
    IDomainManager() = default;
};