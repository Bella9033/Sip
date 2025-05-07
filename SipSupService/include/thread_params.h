// thread_params.h
#pragma once

#include "common.h"
#include "interfaces/isip_task_base.h"
#include <memory>

// 线程处理参数
struct ThRxParams 
{
    pjsip_rx_data* rxdata;
    std::shared_ptr<ISipTaskBase> taskbase;

    ThRxParams() 
        : rxdata(nullptr)
        , taskbase(nullptr) 
    { }
};