// thread_params.h

#pragma once

#include "common.h"
#include "interfaces/isip_task_base.h"
#include <memory>

// 线程处理参数
struct ThRxParams 
{
    // 使用智能指针
    SipTypes::RxDataPtr rxdata;
    std::shared_ptr<ISipTaskBase> taskbase;

    ThRxParams() 
        : rxdata(nullptr)
        , taskbase(nullptr) 
    { }
};