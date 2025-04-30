// thread_params.h
#pragma once

#include "common.h"
#include "sip_task_base.h"
#include <memory>

// 线程处理参数
struct ThRxParams {
    SipTypes::RxDataPtr rxdata;
    std::shared_ptr<SipTaskBase> taskbase;

    ThRxParams() : rxdata(nullptr), taskbase(nullptr) {}
};