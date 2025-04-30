// pjsip_utils.cpp
#include "pjsip_utils.h"


// ===== 资源创建函数实现 =====
// 使用SipTypes工厂函数创建智能指针
SipTypes::CachingPoolPtr PjSipUtils::createCachingPool(pj_size_t sipStackSize) 
{
    LOG(INFO) << "Creating caching pool with size: " << sipStackSize;
    auto pool = new pj_caching_pool();
    if (!pool) return nullptr;
    
    pj_caching_pool_init(pool, nullptr, sipStackSize);
 
    return SipTypes::makeCachingPool(pool);
}

SipTypes::PoolPtr PjSipUtils::createPool(SipTypes::CachingPoolPtr caching_pool, 
    const char* name, pj_size_t initial_size, pj_size_t increment_size) 
{
    if (!caching_pool) return nullptr;
    
    // 使用默认名称，如果没有提供
    const char* pool_name = name ? name : "sip-pool";
    
    pj_pool_t* pool = pj_pool_create(&caching_pool->factory, 
                                     pool_name, 
                                     initial_size, 
                                     increment_size, 
                                     nullptr);
    if (!pool) return nullptr;
 
    return SipTypes::makePool(pool);
}

SipTypes::PoolPtr PjSipUtils::createEndptPool(SipTypes::EndpointPtr endpt,
    const char* name, pj_size_t initial_size, pj_size_t increment_size) 
{
    if (!endpt) return nullptr;
    
    // 使用默认名称，如果没有提供
    const char* pool_name = name ? name : "endpt-pool";
    
    pj_pool_t* pool = pjsip_endpt_create_pool(endpt.get(), 
                                             pool_name, 
                                             initial_size, 
                                             increment_size);
    if (!pool) return nullptr;

    return SipTypes::makePool(pool);
}

SipTypes::TxDataPtr PjSipUtils::createTxData(SipTypes::EndpointPtr endpt)
{
    if (!endpt) return nullptr;
    
    pjsip_tx_data* tdata = nullptr;
    pj_status_t status = pjsip_endpt_create_tdata(endpt.get(), &tdata);
    if (status != PJ_SUCCESS || !tdata) return nullptr;
  
    return SipTypes::makeTxData(tdata);
}

SipTypes::RxDataPtr PjSipUtils::cloneRxData(pjsip_rx_data* raw_data)
{
    if (!raw_data) return nullptr;
    
    pjsip_rx_data* cloned_data = nullptr;
    pj_status_t status = pjsip_rx_data_clone(raw_data, 0, &cloned_data);
    if (status != PJ_SUCCESS || !cloned_data) return nullptr;

    return SipTypes::makeRxData(cloned_data);
}

SipTypes::EndpointPtr PjSipUtils::createEndpoint(SipTypes::CachingPoolPtr caching_pool)
{
    if (!caching_pool) return nullptr;
    
    pjsip_endpoint* endpt = nullptr;
    pj_status_t status = pjsip_endpt_create(&caching_pool->factory, nullptr, &endpt);
    if (status != PJ_SUCCESS || !endpt) return nullptr;

    return SipTypes::makeEndpoint(endpt);
}


// ===== 核心初始化 - 智能指针版本 =====
pj_status_t PjSipUtils::initCore(SipTypes::CachingPoolPtr& caching_pool,
    SipTypes::EndpointPtr& endpt) 
{
    // 创建缓存池（如果未提供）
    if (!caching_pool) 
    {
        caching_pool = createCachingPool();
        if (!caching_pool) 
        {
            LOG(ERROR) << "Failed to create caching pool";
            return PJ_ENOMEM;
        }
    }
    
    pj_status_t status;
    status = pj_init();
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pj_init failed, code: " << status;
        return status;
    }
    
    status = pjlib_util_init();
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjlib_util_init failed, code: " << status;
        return status;
    }

    // 创建终端
    endpt = createEndpoint(caching_pool);
    if (!endpt)
    {
        LOG(ERROR) << "pjsip_endpt_create failed";
        return PJ_ENOMEM;
    }

    status = pjsip_tsx_layer_init_module(endpt.get());
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_tsx_layer_init_module failed, code: " << status;
        endpt = nullptr; // 释放资源
        return status;
    }

    status = pjsip_ua_init_module(endpt.get(), nullptr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_ua_init_module failed, code: " << status;
        endpt = nullptr; // 释放资源
        return status;
    }

    return PJ_SUCCESS;
}


// ===== 传输层初始化 - 智能指针版本 =====
pj_status_t PjSipUtils::initTransports(SipTypes::EndpointPtr endpt, int sip_port) 
{
    if (!endpt) 
    {
        LOG(ERROR) << "Null endpoint provided";
        return PJ_EINVAL;
    }
    
    return initTransports(endpt.get(), sip_port);
}

// pjsip_utils.cpp 中的线程注册函数部分 - 修复版
// ===== 线程注册函数 =====
// 增加线程安全相关的改进
pj_status_t PjSipUtils::registerThread() 
{
    // 修复：每个线程需要专有的描述符
    static thread_local bool registered = false;
    static thread_local pj_thread_desc desc;
    static thread_local pj_thread_t* thread = nullptr;
    
    if (!registered) 
    {
        pj_status_t status = pj_thread_register(nullptr, desc, &thread);
        if (status == PJ_SUCCESS) 
        {
            registered = true;
            return PJ_SUCCESS;
        }
        return status;
    }
    
    LOG(INFO) << "Thread already registered";
    return PJ_SUCCESS;
}


// ===== 线程注册器构造函数 =====
PjSipUtils::ThreadRegistrar::ThreadRegistrar()
{
    pj_status_t status = registerThread();
    if (status != PJ_SUCCESS) 
    {
        LOG(ERROR) << "Thread registration failed, code: " << status;
        throw std::runtime_error("Thread registration failed");  // 修复：失败时抛出异常
    }
}


// ===== 资源清理 - 智能指针版本 =====
void PjSipUtils::cleanupCore(SipTypes::CachingPoolPtr& caching_pool, 
                             SipTypes::EndpointPtr& endpt) 
{
    if (endpt) 
    {
        auto temp_endpt = std::move(endpt);
        endpt = nullptr;
    }
    if (caching_pool) 
    {
        auto temp_pool = std::move(caching_pool);
        caching_pool = nullptr;
    }
    pj_shutdown();
    LOG(INFO) << "PJSIP core cleanup completed";
}




// ===== 核心初始化 - 原始指针版本(向后兼容) =====
pj_status_t PjSipUtils::initCore(pj_caching_pool& caching_pool,
    pjsip_endpoint*& endpt) 
{
    pj_status_t status;

    status = pj_init();
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pj_init failed, code: " << status;
        return status;
    }
    status = pjlib_util_init();
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjlib_util_init failed, code: " << status;
        return status;
    }

    status = pjsip_endpt_create(&caching_pool.factory, nullptr, &endpt);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_endpt_create failed, code: " << status;
        return status;
    }

    status = pjsip_tsx_layer_init_module(endpt);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_tsx_layer_init_module failed, code: " << status;
        return status;
    }

    status = pjsip_ua_init_module(endpt,nullptr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_ua_init_module failed, code: " << status;
        return status;
    }

    return PJ_SUCCESS;
}


// ===== 传输层初始化 - 原始指针版本 =====
pj_status_t PjSipUtils::initTransports(pjsip_endpoint* endpt, int sip_port) 
{
    if (!endpt) {
        LOG(ERROR) << "Null endpoint provided";
        return PJ_EINVAL;
    }
    
    pj_sockaddr_in addr;
    pj_bzero(&addr, sizeof(addr));
    addr.sin_family = pj_AF_INET();
    addr.sin_addr.s_addr = 0;
    addr.sin_port = pj_htons(static_cast<pj_uint16_t>(sip_port));

    pj_status_t status;
    status = pjsip_udp_transport_start(endpt, &addr, nullptr, 1, nullptr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_udp_transport_start failed, code: " << status;
        return status;
    }
    LOG(INFO) << "sip udp:" << sip_port << " is running ...";

    status = pjsip_tcp_transport_start(endpt, &addr, 1, nullptr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_tcp_transport_start failed, code: " << status;
        return status;
    }
    LOG(INFO) << "sip tcp:" << sip_port << " is running ...";

    return PJ_SUCCESS; 
}


// ===== 资源清理 - 裸指针版本 =====
void PjSipUtils::cleanupCoreRaw(pj_caching_pool* caching_pool, pjsip_endpoint* endpt) 
{
    if (endpt) 
    {
        pjsip_endpt_destroy(endpt);
    }
    if (caching_pool) 
    {
        pj_caching_pool_destroy(caching_pool);
    }
    pj_shutdown();
    LOG(INFO) << "PJSIP core cleanup completed (raw pointers)";
}