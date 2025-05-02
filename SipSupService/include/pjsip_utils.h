// pjsip_utils.h
// 负责所有PJSIP资源的创建、管理和释放

#pragma once

#include "common.h"
#include "sip_types.h"

namespace PjSipUtils {
    // ===== 资源创建函数 =====
    // 修改：从SipTypes移动所有资源创建函数到这里
    SipTypes::CachingPoolPtr createCachingPool(pj_size_t sipStackSize = SIP_STACK_SIZE);
    
    SipTypes::PoolPtr createPool(SipTypes::CachingPoolPtr caching_pool, 
        const char* name, pj_size_t initial_size = SIP_ALLOC_POOL_1M, 
        pj_size_t increment_size = SIP_STACK_SIZE);
    
    SipTypes::PoolPtr createEndptPool(SipTypes::EndpointPtr endpt,
        const char* name, pj_size_t initial_size = SIP_ALLOC_POOL_1M, 
        pj_size_t increment_size = SIP_STACK_SIZE);

    SipTypes::TxDataPtr createTxData(SipTypes::EndpointPtr endpt);
    SipTypes::RxDataPtr cloneRxData(pjsip_rx_data* raw_data);
    SipTypes::EndpointPtr createEndpoint(SipTypes::CachingPoolPtr caching_pool);

    // ===== 核心初始化函数 =====
    // 智能指针版本
    pj_status_t initCore(SipTypes::CachingPoolPtr& caching_pool,
                         SipTypes::EndpointPtr& endpt);

    // 兼容原始指针版本（向后兼容）
    pj_status_t initCore(pj_caching_pool& caching_pool, pjsip_endpoint*& endpt);

    // ===== 传输层初始化 =====
    // 智能指针版本
    pj_status_t initTransports(SipTypes::EndpointPtr endpt, int sip_port);
    // 原始指针版本
    pj_status_t initTransports(pjsip_endpoint* endpt, int sip_port);

    // ===== 资源清理 =====
    // 智能指针版本
    void cleanupCore(SipTypes::CachingPoolPtr& caching_pool, SipTypes::EndpointPtr& endpt);
    // 原始指针版本
    void cleanupCoreRaw(pj_caching_pool* caching_pool, pjsip_endpoint* endpt);

    // ===== 线程管理 =====
    pj_status_t registerThread();

    // RAII线程注册器
    class ThreadRegistrar 
    {
    public:
        ThreadRegistrar();
        ~ThreadRegistrar() = default;
    };

} // namespace PjSipUtils