// sip_core.cpp

#include "sip_core.h"
#include "sip_register.h"

std::atomic<bool> SipCore::stop_pool_{false};

pjsip_module SipCore::recv_mod = {
    nullptr, nullptr,
    {const_cast<char*>("mod-recv"), 8}, -1,
    PJSIP_MOD_PRIORITY_APPLICATION,
    nullptr, nullptr, nullptr, nullptr,
    // 在初始化时使用原始指针版本的回调
    &SipCore::onRxRequestRaw,  // 使用原始指针版本
    nullptr, nullptr, nullptr, nullptr
};

void SipCore::pollingEventLoop(SipTypes::EndpointPtr endpt) 
{
    LOG(INFO) << "pollingEventLoop called";
    PjSipUtils::ThreadRegistrar thread_registrar;
    if (!endpt) 
    {
        LOG(ERROR) << "pollingEventLoop received nullptr endpoint!";
        return;
    }
    LOG(INFO) << "pollingEventLoop started";
    while (!stop_pool_) 
    {
        pj_time_val timeout = {0, 500};
        pj_status_t status = pjsip_endpt_handle_events(endpt.get(), &timeout);
        if (status != PJ_SUCCESS)
        {
            LOG(ERROR) << "pollingEventLoop failed, code: " << status;
            return;
        }
    }
    LOG(INFO) << "pollingEventLoop exited normally";
}

pj_bool_t SipCore::onRxRequestRaw(pjsip_rx_data* rdata)
{
    auto rdata_ptr = std::shared_ptr<pjsip_rx_data>(rdata, [](pjsip_rx_data*){});
    return onRxRequest(rdata_ptr);
}

pj_bool_t SipCore::onRxRequest(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "onRxRequest called";
    if (!rdata || !rdata->msg_info.msg) 
    {
        LOG(ERROR) << "rdata or msg_info is null";
        return PJ_FALSE;
    }

    LOG(INFO) << "onRxRequest: " << pjsip_rx_data_get_info(rdata.get());

    return PJ_SUCCESS;
}

SipCore::SipCore()
{
    caching_pool_ = PjSipUtils::createCachingPool(SIP_STACK_SIZE);
    if (!caching_pool_) 
    {
        LOG(ERROR) << "Failed to create caching pool";
    }
    endpt_ = nullptr;
}

SipCore::~SipCore() 
{
    LOG(INFO) << "Releasing SipCore...";
    stop_pool_ = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    PjSipUtils::shutdownCore(caching_pool_, endpt_);
}

pj_status_t SipCore::initSip(int sip_port) 
{  
    pj_log_set_level(0);
    pj_status_t status;

    if (!caching_pool_) 
    {
        LOG(ERROR) << "caching_pool_ is null. Did you forget to initialize it?";
        return PJ_ENOMEM;
    }

    status = PjSipUtils::initCore(caching_pool_, endpt_);
    if (status != PJ_SUCCESS) 
    {
        LOG(ERROR) << "PjSipUtils::initCore failed, code: " << status;
        return status;
    }  

    if (!endpt_) 
    {
        LOG(ERROR) << "endpt_ is null after initialization!";
        return PJ_ENOMEM;
    }

    status = PjSipUtils::initTransports(endpt_, sip_port);
    if (status != PJ_SUCCESS) 
    {
        LOG(ERROR) << "PjSipUtils::initTransports failed, code: " << status;
        return status;
    }
    
    status = pjsip_endpt_register_module(endpt_.get(), &recv_mod);
    if (status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_endpt_register_module failed, code: " << status;
        return status;
    }
    
    pool_ = PjSipUtils::createEndptPool(endpt_, nullptr, SIP_ALLOC_POOL_1M, SIP_ALLOC_POOL_1M);
    if (!pool_)
    {
        LOG(ERROR) << "Failed to create endpoint pool";
        return PJ_ENOMEM;
    }
    
    if (!endpt_) 
    {
        LOG(ERROR) << "endpt_ is nullptr, won't create polling thread!";
        return PJ_ENOMEM;
    }

    auto self = shared_from_this();
    auto endpt_copy = endpt_;
    std::thread th_event([self, endpt_copy]() {
        try {
            self->pollingEventLoop(endpt_copy);
        } catch (const std::exception& e) {
            LOG(ERROR) << "Exception in pollingEventLoop: " << e.what();
        }
    });
    th_event.detach();
    return PJ_SUCCESS;
}