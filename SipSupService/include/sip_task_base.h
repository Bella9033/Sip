#pragma once

#include "common.h"
#include "pjsip_utils.h"

// SIP任务基类接口
class SipTaskBase 
{
public:
    virtual ~SipTaskBase() = default;
    virtual pj_status_t runRxTask(SipTypes::RxDataPtr rdata) = 0;
};

// 注册任务基类
class SipRegTaskBase : public SipTaskBase 
{
public:
    virtual ~SipRegTaskBase() = default;
    virtual pj_status_t registerReqMsg(SipTypes::RxDataPtr rdata) = 0;

protected:
    virtual std::string parseFromHeader(pjsip_msg* msg) = 0;
};