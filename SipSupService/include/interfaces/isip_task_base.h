// isip_task_base.h
#pragma once

#include "common.h"
#include "pjsip_utils.h"

// SIP任务基类接口
class ISipTaskBase 
{
public:
    virtual ~ISipTaskBase() = default;
    virtual pj_status_t runRxTask(pjsip_rx_data* rdata) = 0;

protected:
    virtual std::string parseFromHeader(pjsip_msg* msg) = 0;
};