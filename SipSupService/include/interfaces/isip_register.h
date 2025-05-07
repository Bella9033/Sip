// isip_register.h

#pragma once

#include "common.h"
#include "pjsip_utils.h"

// SIP注册功能接口
class ISipRegister 
{
public:
    virtual ~ISipRegister() = default;
    virtual void startRegService() = 0;
    
protected:
    virtual std::string parseFromHeader(pjsip_msg* msg) = 0;
};