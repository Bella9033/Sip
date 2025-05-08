// pjsip_utils.h
// 负责所有PJSIP资源的创建、管理和释放

#pragma once

#include "common.h"
#include "sip_types.h"

// 不要用智能指针管理 pjsip_tx_data、pjsip_rx_data 等 PJSIP 资源。
// PJSIP 里的许多对象（如 endpoint、pool、txdata）必须保证销毁前没有其他线程再用，否则会因锁对象提前释放或未初始化导致崩溃。
// 你在 SIP 注册/响应流程每次都用智能指针包裹 endpoint/txdata/pool，
// 但这些对象的引用计数失效时就会自动析构，可能被早于底层线程清理，造成互斥锁失效。

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

    std::string getPjStatusString(pj_status_t status);
    
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