#pragma once

#include "common.h"
#include "interfaces/isip_core.h"
#include "interfaces/isip_register.h"
#include "pjsip_utils.h"
#include <atomic>
#include <memory>

// 定义线程参数结构
struct ThRxParams {
    std::shared_ptr<ISipRegister> taskbase;
    SipTypes::RxDataPtr rxdata;
};

class SipCore : public ISipCore,
                public std::enable_shared_from_this<SipCore> {
public:
    static std::shared_ptr<SipCore> create();
    
    explicit SipCore();
    virtual ~SipCore();

    // 删除拷贝和移动
    SipCore(const SipCore&) = delete;
    SipCore& operator=(const SipCore&) = delete;
    SipCore(SipCore&&) = delete;
    SipCore& operator=(SipCore&&) = delete;

    pj_status_t initSip(int sip_port) override;
    SipTypes::EndpointPtr getEndPoint() const override { return endpt_; }

private:
    void pollingEventLoop(SipTypes::EndpointPtr endpt);
    static pj_bool_t onRxRequestRaw(pjsip_rx_data* rdata);
    static pj_bool_t onRxRequest(SipTypes::RxDataPtr rdata);

private:
    static std::atomic<bool> stop_pool_;
    static pjsip_module recv_mod;
    
    SipTypes::CachingPoolPtr caching_pool_;
    SipTypes::EndpointPtr endpt_;
    SipTypes::PoolPtr pool_;
};