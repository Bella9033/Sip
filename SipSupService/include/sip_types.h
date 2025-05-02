// sip_types.h
// 负责类型定义和智能指针包装
#pragma once

#include <memory>
#include "common.h" 

// SIP数据类型的智能指针封装
namespace PjSipUtils {
    // 定义智能指针类型及其删除器
    using TxDataPtr = std::shared_ptr<pjsip_tx_data>;
    using RxDataPtr = std::shared_ptr<pjsip_rx_data>;
    using EndpointPtr = std::shared_ptr<pjsip_endpoint>;
    using CachingPoolPtr = std::shared_ptr<pj_caching_pool>;
    using PoolPtr = std::shared_ptr<pj_pool_t>;
    
    // 删除器定义集中放在这里，保证一致性
    // 修改：将删除器单独定义为结构体，提高可测试性和可维护性
    struct Deleters {
        static void deleteTxData(pjsip_tx_data* p) {
            if (p) pjsip_tx_data_dec_ref(p);
        }
        
        static void deleteRxData(pjsip_rx_data* p) {
            if (p) pjsip_rx_data_free_cloned(p);
        }
        
        static void deleteEndpoint(pjsip_endpoint* p) {
            if (p) pjsip_endpt_destroy(p);
        }
        
        static void deleteCachingPool(pj_caching_pool* p) {
            if (p) {
                pj_caching_pool_destroy(p);
                delete p;  // 注意：这里需要delete，因为我们使用了new
            }
        }
        
        static void deletePool(pj_pool_t* p) {
            if (p) pj_pool_release(p);
        }
    };
    
    // 修改：定义创建智能指针的工厂函数
    inline TxDataPtr makeTxData(pjsip_tx_data* p) {
        return p ? TxDataPtr(p, Deleters::deleteTxData) : nullptr;
    }
    
    inline RxDataPtr makeRxData(pjsip_rx_data* p) {
        return p ? RxDataPtr(p, Deleters::deleteRxData) : nullptr;
    }
    
    inline EndpointPtr makeEndpoint(pjsip_endpoint* p) {
        return p ? EndpointPtr(p, Deleters::deleteEndpoint) : nullptr;
    }
    
    inline CachingPoolPtr makeCachingPool(pj_caching_pool* p) {
        return p ? CachingPoolPtr(p, Deleters::deleteCachingPool) : nullptr;
    }
    
    inline PoolPtr makePool(pj_pool_t* p) {
        return p ? PoolPtr(p, Deleters::deletePool) : nullptr;
    }

}