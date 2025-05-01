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

    virtual std::vector<DomainInfo>& getDomainInfoList() = 0;

    virtual std::shared_mutex& getMutex() = 0; // 获取互斥锁

};