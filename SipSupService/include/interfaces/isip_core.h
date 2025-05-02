// isip_core.h
#pragma once

#include "common.h"
#include "pjsip_utils.h"

#include <memory>

// SIP核心功能接口
class ISipCore 
{
public:
    virtual ~ISipCore() = default;
    virtual pj_status_t initSip(int sip_port) = 0;
    virtual SipTypes::EndpointPtr getEndPoint() const = 0;
    // 添加其他必要的接口方法
};