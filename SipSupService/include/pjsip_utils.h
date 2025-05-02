#pragma once

#include "common.h"
#include <pj/pool.h>
#include <pjsip.h>
#include <memory>

// 前向声明
struct pj_caching_pool;
struct pjsip_endpoint;
struct pj_pool_t;
struct pjsip_rx_data;

namespace SipTypes {
    using CachingPoolPtr = std::shared_ptr<pj_caching_pool>;
    using EndpointPtr = std::shared_ptr<pjsip_endpoint>;
    using PoolPtr = std::shared_ptr<pj_pool_t>;
    using RxDataPtr = std::shared_ptr<pjsip_rx_data>;
}

namespace PjSipUtils {
    SipTypes::CachingPoolPtr createCachingPool(size_t pool_size);
    
    pj_status_t initCore(SipTypes::CachingPoolPtr& caching_pool,
                        SipTypes::EndpointPtr& endpt);
    
    void cleanupCore(SipTypes::CachingPoolPtr& caching_pool,
                    SipTypes::EndpointPtr& endpt);
    
    SipTypes::PoolPtr createEndptPool(const SipTypes::EndpointPtr& endpt,
                                     const char* pool_name,
                                     pj_size_t initial_size,
                                     pj_size_t increment_size);

    // 添加其他需要的工具函数
}