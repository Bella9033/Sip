#pragma once

#include "common.h"
#include "pjsip_utils.h"

// SIP任务基类接口
class SipTaskBase {
public:
    virtual ~SipTaskBase() = default;
    virtual pj_status_t runRxTask(PjSipUtils::RxDataPtr rdata) = 0;
};

// 注册任务基类
class SipRegTaskBase : public SipTaskBase {
public:
    virtual ~SipRegTaskBase() = default;
    virtual pj_status_t registerReqMsg(PjSipUtils::RxDataPtr rdata) = 0;
};