// idomain_manager.h

#pragma once

#include "node_info.h"
#include <string>
#include <string_view>
#include <vector>

// 域名管理接口
class IDomainManager 
{
public:
    virtual ~IDomainManager() = default;
    virtual void buildDomainInfoList() = 0;
    virtual bool checkIsValid(const std::string& id) const = 0;
    virtual void setExpires(std::string_view id, int expires) = 0;
    virtual std::vector<DomainInfo>& getDomainInfoList() = 0;
};