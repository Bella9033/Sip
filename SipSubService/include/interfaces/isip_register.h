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
    
};