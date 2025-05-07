// isip_task_base.h

#pragma once

#include "common.h"
#include "pjsip_utils.h"

// SIP任务基类接口
class ISipTaskBase 
{
public:
    virtual ~ISipTaskBase() = default;
    
    // 使用智能指针作为参数
    virtual pj_status_t runRxTask(SipTypes::RxDataPtr rdata) = 0;
    virtual pj_status_t registerReqMsg(SipTypes::RxDataPtr rdata) = 0;

protected:
    virtual std::string parseFromHeader(pjsip_msg* msg) = 0;
};