// iconfig_provider.h

#pragma once

#include "node_info.h"
#include <string>
#include <vector>

// 配置提供者接口
class IConfigProvider 
{
public:
    virtual ~IConfigProvider() = default;
    virtual const std::string& getLocalIp() const = 0;
    virtual const std::string& getSipId() const = 0;
    virtual const std::string& getSipIp() const = 0;
    virtual int getSipPort() const = 0;
    virtual const std::string& getNodeRealm() const = 0;
    virtual const std::vector<NodeInfo>& getNodeInfoList() const = 0;
    virtual bool readConf() = 0; // 添加读取配置的接口方法
};