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

SipCore::SipCore()
{
    try {
        LOG(INFO) << "Creating caching pool with size: " << SIP_STACK_SIZE;
        caching_pool_ = PjSipUtils::createCachingPool(SIP_STACK_SIZE);
        if (!caching_pool_) 
        {
            LOG(ERROR) << "Failed to create caching pool";
            throw std::runtime_error("Failed to create caching pool");
        }
        LOG(INFO) << "Caching pool created successfully";
    } catch (const std::bad_alloc& e) {
        LOG(ERROR) << "Memory allocation failed in SipCore constructor: " << e.what();
        throw; // 重新抛出异常以便上层捕获
    }
    endpt_ = nullptr;
}

SipCore::~SipCore() 
{
    LOG(INFO) << "Releasing SipCore...";
    stop_pool_ = true;
    
    // 增加足够的等待时间，确保pollingEventLoop安全退出
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 直接使用 PjSipUtils 的清理函数
    PjSipUtils::cleanupCore(caching_pool_, endpt_);
}


void SipCore::pollingEventLoop(SipTypes::EndpointPtr endpt) 
{
    // 需要在这里添加线程注册，因为这是处理SIP消息的主循环线程
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
        // 正确处理超时状态，PJ_ETIMEDOUT是正常的超时返回
        if (status != PJ_SUCCESS && status != PJ_ETIMEDOUT)
        {
            LOG(ERROR) << "pollingEventLoop failed, code: " << status;
            return;
        }
    }
    LOG(INFO) << "pollingEventLoop exited normally";
}


pj_status_t SipCore::initSip(int sip_port) 
{  
    LOG(INFO) << "Initializing SipCore...";
    pj_log_set_level(6);
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
    
    // 添加默认池名称
    pool_ = PjSipUtils::createEndptPool(endpt_, "sipcore-pool", SIP_ALLOC_POOL_1M, SIP_ALLOC_POOL_1M);
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

    // 安全创建线程
    auto self = shared_from_this();
    auto endpt_copy = endpt_;
    
    try {
        // 使用适当超时时间创建线程
        auto thread_future = EVThread::createThread(
            [self, endpt_copy]() {
                try {
                    self->pollingEventLoop(endpt_copy);
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Exception in pollingEventLoop: " << e.what();
                }
            },
            std::tuple<>{},
            nullptr,
            ThreadPriority::NORMAL,
            std::chrono::milliseconds{30000} 
        );
    } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to create polling thread: " << e.what();
        return PJ_EINVAL;
    }
    
    return PJ_SUCCESS;
}


pj_bool_t SipCore::onRxRequestRaw(pjsip_rx_data* rdata)
{
    if (!rdata) 
    {
        LOG(ERROR) << "Received null rdata in onRxRequestRaw";
        return PJ_FALSE;
    }

    // 立即克隆数据并转换为智能指针
    auto rdata_ptr = PjSipUtils::cloneRxData(rdata);
    if (!rdata_ptr) 
    {
        LOG(ERROR) << "Failed to clone rx_data in onRxRequestRaw";
        return PJ_FALSE;
    }
    
    // 调用智能指针版本的处理函数
    return onRxRequest(rdata_ptr);
}

pj_bool_t SipCore::onRxRequest(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "onRxRequest called with rdata=" << (void*)rdata.get();
    // 添加线程注册，因为这是处理接收SIP请求的回调
    PjSipUtils::ThreadRegistrar thread_registrar;

    if (!rdata || !rdata->msg_info.msg) 
    {
        LOG(ERROR) << "rdata or msg_info is null";
        return PJ_FALSE;
    }
    
    
    return PJ_SUCCESS;
}
