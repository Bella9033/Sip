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
    
    // 修复：确保线程已注册到PJSIP
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
        // 修复：正确处理超时状态，PJ_ETIMEDOUT是正常的超时返回
        if (status != PJ_SUCCESS && status != PJ_ETIMEDOUT)
        {
            LOG(ERROR) << "pollingEventLoop failed, code: " << status;
            return;
        }
    }
    LOG(INFO) << "pollingEventLoop exited normally";
}

pj_bool_t SipCore::onRxRequestRaw(pjsip_rx_data* rdata)
{
    if (!rdata) 
    {
        LOG(ERROR) << "Received null rdata in onRxRequestRaw";
        return PJ_FALSE;
    }

    // 立即克隆数据
    auto rdata_ptr = PjSipUtils::cloneRxData(rdata);
    if (!rdata_ptr) 
    {
        LOG(ERROR) << "Failed to clone rx_data in onRxRequestRaw";
        return PJ_FALSE;
    }
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

    // 创建参数对象
    auto params = std::make_shared<ThRxParams>();
    
    // 这里不需要再次克隆，直接使用传入的智能指针
    params->rxdata = rdata;

    // 根据方法ID创建适当的任务处理器
    if (rdata->msg_info.msg->line.req.method.id == PJSIP_REGISTER_METHOD) 
    { 
        params->taskbase = SipRegister::createInstance();
    }
    else
    {
        // 修复：添加对其他方法的处理，以便系统更完整
        LOG(WARNING) << "Unknown or unsupported request method ID: " 
                     << rdata->msg_info.msg->line.req.method.id;
        return PJ_FALSE;
    }
    
    // 修复：确保参数在线程执行期间有效
    auto params_copy = params; // 复制shared_ptr，确保引用计数增加
    
    auto worker = [params_copy]() -> int {
        // 修复：确保线程注册到PJSIP
        PjSipUtils::ThreadRegistrar registrar;
        
        if(!params_copy || !params_copy->taskbase) 
        {
            LOG(ERROR) << "params or taskbase null";
            return -1;
        }
        
        // 修复：增加错误处理
        try {
            params_copy->taskbase->runRxTask(params_copy->rxdata);
            LOG(INFO) << "runRxTask success in PJSIP_REGISTER_METHOD";
            return 0;
        } catch (const std::exception& e) {
            LOG(ERROR) << "Exception in runRxTask: " << e.what();
            return -1;
        }
    };

    try {
        // 修改：增加错误处理
        auto fut = EVThread::createThread(
            std::move(worker), 
            std::tuple<>(), 
            nullptr, 
            ThreadPriority::NORMAL, 
            std::chrono::milliseconds{5000} // 超时时间
        );

        int result = fut.get();
        if(result != 0) 
        {
            LOG(ERROR) << "Thread execution failed with result: " << result;
            return PJ_FALSE;
        }
        return PJ_TRUE; // 成功返回PJ_TRUE而非PJ_SUCCESS
    } catch (const std::exception& e) {
        LOG(ERROR) << "Exception in thread creation: " << e.what();
        return PJ_FALSE;
    }
}

SipCore::SipCore()
{
    caching_pool_ = PjSipUtils::createCachingPool(SIP_STACK_SIZE);
    if (!caching_pool_) 
    {
        LOG(ERROR) << "Failed to create caching pool";
        throw std::runtime_error("Failed to create caching pool");
    }
    endpt_ = nullptr;
}

SipCore::~SipCore() 
{
    LOG(INFO) << "Releasing SipCore...";
    stop_pool_ = true;
    
    // 修复：增加足够的等待时间，确保pollingEventLoop安全退出
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 直接使用 PjSipUtils 的清理函数
    PjSipUtils::cleanupCore(caching_pool_, endpt_);
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
    
    // 修复：添加默认池名称
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

    // 修复：安全创建线程
    auto self = shared_from_this();
    auto endpt_copy = endpt_;
    
    try {
        // 修复：使用适当超时时间创建线程
        auto thread_future = EVThread::createThread(
            [self, endpt_copy]() {
                try {
                    self->pollingEventLoop(endpt_copy);
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Exception in pollingEventLoop: " << e.what();
                }
            },
            std::tuple<>(),
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