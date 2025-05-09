// sip_core.h
// 调用 PjSipUtils 提供的工具函数，并保留业务事件循环与请求分发逻辑。

#pragma once

#include <atomic>
#include <memory>

#include "common.h"
#include "pjsip_utils.h"
#include "ev_thread.h"

#include "interfaces/isip_core.h"
#include "interfaces/idomain_manager.h"

#include "thread_params.h" // 线程参数类

// 前向声明
class SipRegTaskBase;
class SipRegister;
class IDomainManager;
class GlobalCtl;

// 实现ISipCore接口
class SipCore : public ISipCore, 
                public std::enable_shared_from_this<SipCore>
{
public:
    SipCore();
    ~SipCore();

    // 实现ISipCore接口
    pj_status_t initSip(int sip_port) override;
    SipTypes::EndpointPtr getEndPoint() const override { return endpt_; }
    
    void pollingEventLoop(SipTypes::EndpointPtr endpt);

    // 修改函数声明，使用智能指针参数
    static pj_bool_t onRxRequest(SipTypes::RxDataPtr rdata);
    
    // 保持原有的裸指针版本，作为外部回调接口
    static pj_bool_t onRxRequestRaw(pjsip_rx_data* rdata);

    static std::atomic<bool> stop_pool_;
    static pjsip_module recv_mod;

    
private:

    SipTypes::CachingPoolPtr caching_pool_;
    SipTypes::EndpointPtr endpt_;
    SipTypes::PoolPtr pool_;


};