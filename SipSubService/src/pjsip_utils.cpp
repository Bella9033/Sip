// pjsip_utils.cpp

#include "pjsip_utils.h"

// 核心初始化 - 原始指针版本(向后兼容)
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

// 核心初始化 - 智能指针版本
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
    pjsip_endpoint* raw_endpt = nullptr;
    status = pjsip_endpt_create(&caching_pool.get()->factory, nullptr, &raw_endpt);
    if(status != PJ_SUCCESS || !raw_endpt)
    {
        LOG(ERROR) << "pjsip_endpt_create failed, code: " << status;
        return status;
    }

    // 使用自定义删除器包装原始指针
    endpt = SipTypes::EndpointPtr(raw_endpt, [](pjsip_endpoint* p) {
        if (p) pjsip_endpt_destroy(p);
    });

    status = pjsip_tsx_layer_init_module(raw_endpt);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_tsx_layer_init_module failed, code: " << status;
        endpt = nullptr; // 释放资源
        return status;
    }

    status = pjsip_ua_init_module(raw_endpt, nullptr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_ua_init_module failed, code: " << status;
        endpt = nullptr; // 释放资源
        return status;
    }

    return PJ_SUCCESS;
}

// 传输层初始化 - 原始指针版本
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

// 传输层初始化 - 智能指针版本
pj_status_t PjSipUtils::initTransports(SipTypes::EndpointPtr endpt, int sip_port) 
{
    if (!endpt) 
    {
        LOG(ERROR) << "Null endpoint provided";
        return PJ_EINVAL;
    }
    
    return initTransports(endpt.get(), sip_port);
}

pj_status_t PjSipUtils::registerThread() 
{
    pj_thread_desc desc;
    pj_thread_t* thread = nullptr;
    if (!pj_thread_is_registered()) 
    {
        return pj_thread_register(nullptr, desc, &thread);
    }
    LOG(INFO) << "Thread already registered";
    return PJ_SUCCESS;
}

PjSipUtils::ThreadRegistrar::ThreadRegistrar()
{
    pj_status_t status = registerThread();
    if (status != PJ_SUCCESS) 
    {
        LOG(ERROR) << "Thread registration failed, code: " << status;
    }
}

void PjSipUtils::shutdownCore(pj_caching_pool* caching_pool, pjsip_endpoint* endpt) 
{
    if (endpt) {
        pjsip_endpt_destroy(endpt);
    }
    
    if (caching_pool) {
        pj_caching_pool_destroy(caching_pool);
    }
    
    pj_shutdown();
    LOG(INFO) << "PJSIP core shutdown completed";
}

// 智能指针版本的shutdownCore
void PjSipUtils::shutdownCore(SipTypes::CachingPoolPtr& caching_pool, SipTypes::EndpointPtr& endpt) 
{
    endpt = nullptr;
    caching_pool = nullptr;
    
    pj_shutdown();
    LOG(INFO) << "PJSIP core shutdown completed";
}

// 实现创建资源的工厂函数
SipTypes::CachingPoolPtr PjSipUtils::createCachingPool(pj_size_t sipStackSize) 
{
    LOG(INFO) << "Creating caching pool with size: " << sipStackSize;
    auto pool = new pj_caching_pool();
    if (!pool) return nullptr;
    
    pj_caching_pool_init(pool, nullptr, sipStackSize);
    
    // 使用自定义删除器创建智能指针
    return SipTypes::CachingPoolPtr(pool, [](pj_caching_pool* p) {
        if (p) {
            pj_caching_pool_destroy(p);
            delete p;
        }
    });
}

SipTypes::PoolPtr PjSipUtils::createPool(SipTypes::CachingPoolPtr caching_pool, 
    const char* name, pj_size_t initial_size,  pj_size_t increment_size) 
{
    if (!caching_pool) return nullptr;
    
    pj_pool_t* pool = pj_pool_create(&caching_pool.get()->factory, name, initial_size, increment_size, nullptr);
    if (!pool) return nullptr;
    
    // 使用自定义删除器处理资源释放
    return SipTypes::PoolPtr(pool, [](pj_pool_t* p) {
        if (p) pj_pool_release(p);
    });
}

SipTypes::PoolPtr PjSipUtils::createEndptPool(SipTypes::EndpointPtr endpt,
    const char* name, pj_size_t initial_size, pj_size_t increment_size) 
{
    if (!endpt) return nullptr;
    
    pj_pool_t* pool = pjsip_endpt_create_pool(endpt.get(), name, initial_size, increment_size);
    if (!pool) return nullptr;
    
    // 使用自定义删除器处理资源释放
    return SipTypes::PoolPtr(pool, [](pj_pool_t* p) {
        if (p) pj_pool_release(p);
    });
}