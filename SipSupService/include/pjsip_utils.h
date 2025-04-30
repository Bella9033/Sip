#pragma once

#include "common.h"
#include "sip_types.h"

namespace PjSipUtils 
{
// 创建和初始化函数
SipTypes::CachingPoolPtr createCachingPool(pj_size_t sipStackSize = SIP_STACK_SIZE);

SipTypes::PoolPtr createPool(SipTypes::CachingPoolPtr cachingPool, 
    const char* name, pj_size_t initial_size, pj_size_t increment_size);

SipTypes::PoolPtr createEndptPool(SipTypes::EndpointPtr endpt,
    const char* name, pj_size_t initial_size, pj_size_t increment_size);

// 核心初始化 - 智能指针版本
pj_status_t initCore(SipTypes::CachingPoolPtr& caching_pool,
    SipTypes::EndpointPtr& endpt);

// 传输层初始化 - 智能指针版本
pj_status_t initTransports(SipTypes::EndpointPtr endpt, int sip_port);

// 清理 - 智能指针版本
void shutdownCore(SipTypes::CachingPoolPtr& caching_pool, 
    SipTypes::EndpointPtr& endpt);

// 线程注册包装
pj_status_t registerThread();

// 兼容原始指针接口
pj_status_t initCore(pj_caching_pool& caching_pool,
                     pjsip_endpoint*& endpt);

pj_status_t initTransports(pjsip_endpoint* endpt, int sip_port);

void shutdownCore(pj_caching_pool* caching_pool, pjsip_endpoint* endpt);

// RAII：构造时注册线程
class ThreadRegistrar 
{
public:
    ThreadRegistrar();
    ~ThreadRegistrar() = default;
};

} // namespace PjSipUtils